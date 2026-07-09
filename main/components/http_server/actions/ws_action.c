#include "esp_http_server.h"
#include "esp_log.h"
#include <stdlib.h>
#include <string.h>

static const char* WS_ACTION_TAG = "WS_A";

/* Один подключённый клиент (камера обслуживает одного зрителя). */
static httpd_handle_t s_ws_hd = NULL;
static int s_ws_fd = -1;

/* Отправить текстовый фрейм подключённому клиенту (send-test).
 * Потокобезопасно: httpd_ws_send_frame_async ставит работу в очередь worker'а.
 */
esp_err_t ws_send_event(const char* text)
{
  if (s_ws_hd == NULL || s_ws_fd < 0) {
    ESP_LOGW(WS_ACTION_TAG, "no ws client connected");
    return ESP_FAIL;
  }

  httpd_ws_frame_t frame = {
      .type = HTTPD_WS_TYPE_TEXT,
      .payload = (uint8_t*)text,
      .len = strlen(text),
  };
  return httpd_ws_send_frame_async(s_ws_hd, s_ws_fd, &frame);
}

/* Вызывается httpd сразу после успешного рукопожатия (сам upgrade httpd
 * обрабатывает внутри и uri-handler на нём НЕ вызывает). Здесь запоминаем
 * сокет и шлём приветственный кадр — server-push (send-test). */
esp_err_t ws_post_handshake(httpd_req_t* req)
{
  s_ws_hd = req->handle;
  s_ws_fd = httpd_req_to_sockfd(req);
  ESP_LOGI(WS_ACTION_TAG, "ws client connected fd=%d", s_ws_fd);
  ws_send_event("hello");
  return ESP_OK;
}

esp_err_t ws_handler(httpd_req_t* req)
{
  /* Сюда попадают только data-кадры от уже подключённого клиента. */
  /* Определяем длину входящего фрейма. */
  httpd_ws_frame_t frame = {.type = HTTPD_WS_TYPE_TEXT};
  esp_err_t err = httpd_ws_recv_frame(req, &frame, 0);
  if (err != ESP_OK) {
    ESP_LOGW(WS_ACTION_TAG, "recv len probe failed: %s", esp_err_to_name(err));
    return err;
  }

  if (frame.len == 0) {
    return ESP_OK;
  }

  uint8_t* buf = calloc(1, frame.len + 1);
  if (buf == NULL) {
    ESP_LOGE(WS_ACTION_TAG, "oom for %u byte frame", (unsigned)frame.len);
    return ESP_ERR_NO_MEM;
  }
  frame.payload = buf;

  err = httpd_ws_recv_frame(req, &frame, frame.len);
  if (err == ESP_OK) {
    ESP_LOGI(WS_ACTION_TAG, "echo %u bytes", (unsigned)frame.len);
    err = httpd_ws_send_frame(req, &frame); /* echo (receive-test) */
  } else {
    ESP_LOGW(WS_ACTION_TAG, "recv payload failed: %s", esp_err_to_name(err));
  }

  free(buf);
  return err;
}
