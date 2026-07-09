#include <esp_timer.h>

#include "components/config/config.h"
#include <esp_wifi.h>

#include "components/camera/camera.h"
#include "esp_http_server.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "img_converters.h"
#include <lwip/sockets.h>
#include <stdio.h>
#include <sys/select.h>

static const char* TAG = "STREAM_A";

#define FPS_SAMPLES 12
#define STREAM_TARGET_FPS 18
#define STREAM_TARGET_MS (1000 / STREAM_TARGET_FPS)

/* ──────────────────────────────────────────────────────────────────────────
 * Адаптивный контроллер JPEG-качества.
 *
 * Проблема: при движении в кадре JPEG становится больше (больше изменившихся
 * пикселей → меньше степень сжатия). Кадр 5KB→8KB при том же TARGET_MS
 * означает в 1.6x больше данных → TCP окно заполняется → фриз.
 *
 * Решение: PI-регулятор удерживает размер кадра около TARGET_BYTES,
 * автоматически увеличивая сжатие при движении и снижая — в статике.
 * Качество меняется на ±1 шаг за кадр → плавная, без осцилляций реакция.
 * ────────────────────────────────────────────────────────────────────────── */
#define AQC_TARGET_BYTES \
  28000 /* целевой размер кадра в байтах (SVGA, хорошее качество) */
#define AQC_QUALITY_MIN 6 /* лучшее качество (меньше = крупнее файл) */
#define AQC_QUALITY_MAX \
  16 /* худшее качество при максимальном движении (не блочно)*/
#define AQC_STEP_UP 2 /* шаг ухудшения при большом кадре (быстро) */
#define AQC_STEP_DOWN 1 /* шаг улучшения при малом кадре (медленно) */
#define AQC_HYSTERESIS 4000 /* мёртвая зона ±4000 байт (нет смысла дёргать)*/

typedef struct
{
  int64_t buf[FPS_SAMPLES];
  uint8_t idx;
  uint8_t cnt;
  int64_t sum;
} fps_filter_t;

static void fps_reset(fps_filter_t* f)
{
  memset(f, 0, sizeof(*f));
}

static float fps_update(fps_filter_t* f, int64_t ms)
{
  f->sum -= f->buf[f->idx];
  f->buf[f->idx] = ms;
  f->sum += ms;
  f->idx = (f->idx + 1) % FPS_SAMPLES;
  if (f->cnt < FPS_SAMPLES) {
    f->cnt++;
  }
  const int64_t avg = f->sum / f->cnt;
  return avg > 0 ? 1000.0f / (float)avg : 0.0f;
}

/* Тело стриминга выполняется в отдельной задаче, чтобы не блокировать
 * единственный worker httpd на всё время трансляции (async-handler). */
static void stream_task(void* arg)
{
  httpd_req_t* req = (httpd_req_t*)arg;
  /* ── Настройка сокета ── */
  const int fd = httpd_req_to_sockfd(req);
  if (fd >= 0) {
    const int flag = 1;
    const int sndbuf = 65535;
    /* Отключить Nagle: каждый send() уходит в сеть немедленно */
    setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(flag));
    /* Увеличить TX-буфер */
    setsockopt(fd, SOL_SOCKET, SO_SNDBUF, &sndbuf, sizeof(sndbuf));
    /* Ограничить блокировку send() — страховка от зависа на fade */
    struct timeval sndto = {.tv_sec = 1, .tv_usec = 0};
    setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &sndto, sizeof(sndto));
    /* Keepalive: обнаружим разрыв за ~15 сек */
    setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, &flag, sizeof(flag));
#ifdef TCP_KEEPIDLE
    int idle = 10, intvl = 2, cnt = 3;
    setsockopt(fd, IPPROTO_TCP, TCP_KEEPIDLE, &idle, sizeof(idle));
    setsockopt(fd, IPPROTO_TCP, TCP_KEEPINTVL, &intvl, sizeof(intvl));
    setsockopt(fd, IPPROTO_TCP, TCP_KEEPCNT, &cnt, sizeof(cnt));
#endif
  }

  /* ── Заголовки ответа ── */
  esp_err_t res = httpd_resp_set_type(req, STREAM_CONTENT_TYPE);
  if (res != ESP_OK) {
    httpd_req_async_handler_complete(req);
    vTaskDelete(NULL);
    return;
  }
  httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
  httpd_resp_set_hdr(
      req, "Cache-Control", "no-store, no-cache, must-revalidate");
  httpd_resp_set_hdr(req, "Pragma", "no-cache");

  /* ── Статистика ── */
  fps_filter_t fps;
  fps_reset(&fps);
  int64_t last_ts = esp_timer_get_time();
  int64_t frame_start = last_ts; /* момент начала текущего кадра */
  uint32_t frame_cnt = 0;
  size_t last_size = 0;
  int fail_cnt = 0;
  uint32_t dropped = 0; /* кадров пропущено из-за занятого сокета */

  char part_buf[96]; /* boundary + part-zagolovok odnim send() */

  /* ── Адаптивный контроллер качества: стартуем с умеренного значения ── */
  sensor_t* sensor = esp_camera_sensor_get();
  int aqc_q = 12; /* текущее значение quality           */
  if (sensor) {
    sensor->set_quality(sensor, aqc_q);
  }

  ESP_LOGI(
      TAG,
      "stream: client connected fd=%d  target=%d fps (%dms/frame)  aqc_q=%d",
      fd,
      STREAM_TARGET_FPS,
      STREAM_TARGET_MS,
      aqc_q);

  /* Порог "медленного" кадра для детальной диагностики */
#define SLOW_FRAME_MS (STREAM_TARGET_MS * 3) /* >120ms считается аномалией */

  /* Накопители для min/max за окно */
  int64_t win_cam_max = 0, win_net_max = 0, win_total_max = 0;
  int64_t win_cam_sum = 0, win_net_sum = 0;
  int64_t rssi_log_ts = esp_timer_get_time();

  /* ── Основной цикл стриминга ── */
  while (true) {

    /* ── FPS-ограничитель ────────────────────────────────────────────── */
    {
      int64_t elapsed = (esp_timer_get_time() - frame_start) / 1000;
      if (elapsed < (int64_t)STREAM_TARGET_MS) {
        vTaskDelay(pdMS_TO_TICKS(STREAM_TARGET_MS - elapsed));
      }
    }
    frame_start = esp_timer_get_time();

    /* ── Фаза 1: захват кадра камерой ───────────────────────────────── */
    const int64_t t0 = esp_timer_get_time();
    camera_fb_t* fb = esp_camera_fb_get();
    const int64_t cam_ms = (esp_timer_get_time() - t0) / 1000;

    if (!fb) {
      if (++fail_cnt > 10) {
        ESP_LOGE(TAG, "[DIAG] Camera: 10 consecutive failures — abort");
        break;
      }
      ESP_LOGW(TAG,
               "[DIAG] Camera: fb_get failed #%d  cam_wait=%lldms",
               fail_cnt,
               cam_ms);
      vTaskDelay(pdMS_TO_TICKS(50));
      continue;
    }
    fail_cnt = 0;

    if (fb->format != PIXFORMAT_JPEG) {
      ESP_LOGW(TAG, "[DIAG] Camera: unexpected fmt=%d, skip", fb->format);
      esp_camera_fb_return(fb);
      continue;
    }
    last_size = fb->len;

    /* ── Drop-кадр, если сокет не готов принять (link stalled) ──────────
     * Вместо блокировки send() на секунды при потере пакетов — пропускаем
     * кадр и берём свежий на следующей итерации (grab_mode=LATEST). Stream
     * остаётся «живым»: короткие пропуски вместо многосекундных фризов. */
    if (fd >= 0) {
      fd_set wfds;
      FD_ZERO(&wfds);
      FD_SET(fd, &wfds);
      struct timeval tv = {.tv_sec = 0, .tv_usec = 150000}; /* 150 ms */
      if (select(fd + 1, NULL, &wfds, NULL, &tv) <= 0) {
        esp_camera_fb_return(fb);
        dropped++;
        continue;
      }
    }

    /* ── Фаза 2: отправка по TCP ─────────────────────────────────────── */
    int64_t t1 = esp_timer_get_time();

    /* Granica + part-zagolovok odnim chunkom -> menshe TCP-segmentov */
    int hlen = snprintf(
        part_buf, sizeof(part_buf), STREAM_BOUNDARY STREAM_PART_FMT, fb->len);
    res = httpd_resp_send_chunk(req, part_buf, hlen);
    if (res != ESP_OK) {
      esp_camera_fb_return(fb);
      break;
    }

    res = httpd_resp_send_chunk(req, (const char*)fb->buf, (ssize_t)fb->len);
    esp_camera_fb_return(fb);

    const int64_t net_ms = (esp_timer_get_time() - t1) / 1000;
    const int64_t total_ms = (esp_timer_get_time() - frame_start) / 1000;

    if (res != ESP_OK) {
      break;
    }

    /* ── Детальный лог медленных кадров ─────────────────────────────────
     * Ключ к разгадке: если net_ms >> cam_ms — проблема в WiFi/TCP
     *                  если cam_ms >> net_ms — проблема в камере/DMA
     * ────────────────────────────────────────────────────────────────── */
    if (total_ms > SLOW_FRAME_MS) {
      /* Определяем узкое место */
      const char* bottleneck = (net_ms > cam_ms * 3) ? "NET"
          : (cam_ms > net_ms * 3)                    ? "CAM"
                                                     : "MIX";
      ESP_LOGW(TAG,
                "[SLOW#%lu] total=%lldms  cam=%lldms  net=%lldms  "
                "size=%zuB  q=%d  bottleneck=%s",
                (unsigned long)frame_cnt, total_ms, cam_ms, net_ms,
                last_size, aqc_q, bottleneck);
    }

    /* Накапливаем статистику для периодического отчёта */
    win_cam_sum += cam_ms;
    win_net_sum += net_ms;
    if (cam_ms > win_cam_max) {
      win_cam_max = cam_ms;
    }
    if (net_ms > win_net_max) {
      win_net_max = net_ms;
    }
    if (total_ms > win_total_max) {
      win_total_max = total_ms;
    }

    /* ── AQC ──────────────────────────────────────────────────────────── */
    if (sensor) {
      int new_q = aqc_q;
      int delta_bytes = (int)last_size - AQC_TARGET_BYTES;
      if (delta_bytes > AQC_HYSTERESIS && aqc_q < AQC_QUALITY_MAX) {
        new_q = aqc_q + AQC_STEP_UP;
      } else if (delta_bytes < -AQC_HYSTERESIS && aqc_q > AQC_QUALITY_MIN) {
        new_q = aqc_q - AQC_STEP_DOWN;
      }
      if (new_q != aqc_q) {
        aqc_q = new_q;
        sensor->set_quality(sensor, aqc_q);
      }
    }

    /* ── Периодический сводный отчёт (каждые 60 кадров) ─────────────── */
    const int64_t now = esp_timer_get_time();
    const int64_t delta_ms = (now - last_ts) / 1000;
    last_ts = now;
    const float fps_val = fps_update(&fps, delta_ms);
    frame_cnt++;

    if (frame_cnt % 60 == 0) {
      /* WiFi RSSI подключённого клиента (AP mode) */
      int8_t rssi = 0;
      wifi_sta_list_t sta_list = {0};
      if (esp_wifi_ap_get_sta_list(&sta_list) == ESP_OK && sta_list.num > 0) {
        rssi = sta_list.sta[0].rssi;
      }

      uint32_t win_frames = 60;
      ESP_LOGI(TAG,
                "[STAT] %.1f fps | frame=%zuB | q=%d | drop=%lu | "
                "cam avg=%lldms max=%lldms | "
                "net avg=%lldms max=%lldms | "
                "worst_frame=%lldms | rssi=%ddBm",
                fps_val, last_size, aqc_q, (unsigned long)dropped,
                win_cam_sum / win_frames, win_cam_max,
                win_net_sum / win_frames, win_net_max,
                win_total_max, (int)rssi);

      /* Сбрасываем накопители */
      win_cam_max = win_net_max = win_total_max = 0;
      win_cam_sum = win_net_sum = 0;
      dropped = 0;
    }

    /* ── RSSI каждые 5 секунд между отчётами ────────────────────────── */
    if ((now - rssi_log_ts) > 5000000LL) {
      rssi_log_ts = now;
      wifi_sta_list_t sl = {0};
      if (esp_wifi_ap_get_sta_list(&sl) == ESP_OK && sl.num > 0) {
        const int8_t r = sl.sta[0].rssi;
        const char* quality = r > -50 ? "excellent"
            : r > -65                 ? "good"
            : r > -75                 ? "fair"
            : r > -85                 ? "poor"
                                      : "critical";
        if (r < -65) {
          /* Слабый сигнал — это вероятная причина фризов */
          ESP_LOGW(TAG,
                   "[WIFI] RSSI=%ddBm (%s) — weak signal may cause freezes!",
                   (int)r,
                   quality);
        } else {
          ESP_LOGI(TAG, "[WIFI] RSSI=%ddBm (%s)", (int)r, quality);
        }
      }
    }
  }

  ESP_LOGI(TAG,
           "stream: client disconnected (frames=%lu)",
           (unsigned long)frame_cnt);
  (void)res;
  httpd_req_async_handler_complete(req);
  vTaskDelete(NULL);
}

/* Диспетчер: снимает запрос с worker'а httpd и передаёт его в отдельную
 * задачу. Worker освобождается сразу и может обслуживать другие запросы
 * (/capture, /control, /status, /ws) во время трансляции. */
esp_err_t stream_handler(httpd_req_t* req)
{
  httpd_req_t* copy = NULL;
  esp_err_t err = httpd_req_async_handler_begin(req, &copy);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "async begin failed: %s", esp_err_to_name(err));
    return err;
  }

  if (xTaskCreate(stream_task, "stream", 8192, copy, 5, NULL) != pdPASS) {
    ESP_LOGE(TAG, "failed to spawn stream task");
    httpd_req_async_handler_complete(copy);
    return ESP_FAIL;
  }

  return ESP_OK;
}
