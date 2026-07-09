//
// On-demand MJPEG/AVI recording to SD + Telegram upload.
//

#include "video.h"
#include "avi_writer.h"

#include <stdatomic.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "esp_camera.h"
#include "esp_heap_caps.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/idf_additions.h"
#include "freertos/task.h"

#include "components/config/config.h"
#include "components/http_server/actions/actions.h"  // send_sse
#include "components/sdcard/sdcard.h"
#include "components/telegram/telegram.h"

static const char* VIDEO_TAG = "VIDEO";
static atomic_bool s_recording = ATOMIC_VAR_INIT(false);

typedef struct
{
  int sec;
  char path[80];
} rec_ctx_t;

bool video_is_recording(void)
{
  return atomic_load(&s_recording);
}

static void build_filename(char* out, size_t sz)
{
  time_t now = 0;
  struct tm ti = {0};
  time(&now);
  localtime_r(&now, &ti);
  if (ti.tm_year >= (2020 - 1900)) {
    strftime(out, sz, SD_MOUNT_POINT "/vid_%Y%m%d_%H%M%S.avi", &ti);
  } else {
    static uint32_t idx = 0;
    snprintf(out, sz, SD_MOUNT_POINT "/vid%05lu.avi", (unsigned long)idx++);
  }
}

static void record_task(void* arg)
{
  rec_ctx_t* ctx = (rec_ctx_t*)arg;

  sensor_t* s = esp_camera_sensor_get();
  const framesize_t prev = s ? s->status.framesize : FRAME_SIZE;
  if (s) {
    s->set_framesize(s, VIDEO_FRAME_SIZE);
  }
  // warm-up: discard a couple of frames after the resolution switch
  for (int i = 0; i < 2; i++) {
    camera_fb_t* fb = esp_camera_fb_get();
    if (fb) {
      esp_camera_fb_return(fb);
    }
  }

  avi_writer_t w;
  const uint32_t max_frames = (uint32_t)VIDEO_MAX_SEC * VIDEO_MAX_FPS;
  if (avi_open(&w, ctx->path, 800, 600, max_frames)
      != ESP_OK) {  // must match VIDEO_FRAME_SIZE (FRAMESIZE_SVGA = 800x600)
    ESP_LOGE(VIDEO_TAG, "avi_open failed for %s", ctx->path);
    if (s) {
      s->set_framesize(s, prev);
    }
    send_sse("video", "error");
    free(ctx);
    atomic_store(&s_recording, false);
    vTaskDeleteWithCaps(NULL);
    return;
  }

  const int64_t t_start = esp_timer_get_time();
  const int64_t t_end = t_start + (int64_t)ctx->sec * 1000000;
  int consec_fail = 0;
  while (esp_timer_get_time() < t_end) {
    camera_fb_t* fb = esp_camera_fb_get();
    if (!fb) {
      if (++consec_fail >= 10) {
        ESP_LOGE(VIDEO_TAG, "too many failed grabs, stopping");
        break;
      }
      continue;
    }
    consec_fail = 0;
    const esp_err_t we = avi_write_frame(&w, fb->buf, fb->len);
    esp_camera_fb_return(fb);
    if (we != ESP_OK) {
      ESP_LOGW(VIDEO_TAG, "stop: write/cap limit (%s)", esp_err_to_name(we));
      break;
    }
  }

  const float elapsed = (esp_timer_get_time() - t_start) / 1000000.0f;
  const float fps =
      (elapsed > 0.0f && w.frame_count > 0) ? (w.frame_count / elapsed) : 1.0f;
  avi_finalize(&w, fps);
  ESP_LOGI(VIDEO_TAG,
           "recorded %lu frames in %.1fs (%.1f fps) -> %s",
           (unsigned long)w.frame_count,
           elapsed,
           fps,
           ctx->path);

  if (s) {
    s->set_framesize(s, prev);  // restore stills resolution
  }

  char caption[96];
  snprintf(caption, sizeof(caption), "%s video %.0fs 🎥", APP_NAME, elapsed);
  if (telegram_send_video_file(ctx->path, caption) == ESP_OK) {
    send_sse("video", "sent");
  } else {
    ESP_LOGE(VIDEO_TAG, "Telegram upload failed (file kept on SD)");
    send_sse("video", "upload_failed");
  }

  free(ctx);
  atomic_store(&s_recording, false);
  vTaskDelete(NULL);
}

esp_err_t video_record_start(int sec, char* out_path, size_t out_sz)
{
  if (sec < VIDEO_MIN_SEC) {
    sec = VIDEO_DEFAULT_SEC;
  }
  if (sec > VIDEO_MAX_SEC) {
    sec = VIDEO_MAX_SEC;
  }

  if (!sdcard_is_mounted()) {
    return ESP_ERR_NOT_FOUND;
  }
  if (atomic_exchange(&s_recording, true)) {
    return ESP_ERR_INVALID_STATE;  // already recording
  }

  rec_ctx_t* ctx = malloc(sizeof(*ctx));
  if (!ctx) {
    atomic_store(&s_recording, false);
    return ESP_ERR_NO_MEM;
  }
  ctx->sec = sec;
  build_filename(ctx->path, sizeof(ctx->path));
  if (out_path && out_sz) {
    snprintf(out_path, out_sz, "%s", ctx->path);
  }

  // TLS upload runs inside this task -> generous 28KB stack. Allocate it in
  // PSRAM (xTaskCreateWithCaps): the internal DRAM heap is too fragmented to
  // hand out a 28KB contiguous block once WiFi + camera are up, and this task
  // never disables the flash cache, so an external-RAM stack is safe.
  if (xTaskCreateWithCaps(
          record_task, "video_rec", 28672, ctx, 5, NULL, MALLOC_CAP_SPIRAM)
      != pdPASS)
  {
    ESP_LOGE(VIDEO_TAG,
             "Failed to create record task (free_int=%u largest_int=%u)",
             (unsigned)heap_caps_get_free_size(MALLOC_CAP_INTERNAL),
             (unsigned)heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL));
    free(ctx);
    atomic_store(&s_recording, false);
    return ESP_FAIL;
  }
  return ESP_OK;
}
