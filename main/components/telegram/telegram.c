#include "telegram.h"

#include <esp_heap_caps.h>
#include <esp_log.h>
#include <string.h>

#include "components/bus/bus.h"
#include "components/config/config.h"
#include "components/http_client/http_client.h"
static const char* TELEGRAM_TAG = "TELEGRAM";

esp_err_t telegram_send_text(const char* text)
{
  static char response[512];
  char url[256];
  snprintf(
      url, sizeof(url), "https://api.telegram.org/bot%s/sendMessage", TG_TOKEN);

  char body[512];
  snprintf(body,
           sizeof(body),
           "{\"chat_id\":\"%s\",\"text\":\"%s\"}",
           TG_CHAT_ID,
           text);
  const esp_err_t ret = http_post_req(
      url, body, strlen(body), "application/json", response, sizeof(response));
  ESP_LOGI(TELEGRAM_TAG, "Response: %s", response);

  return ret;
}

void telegram_send_text_async(char* text)
{
  char* owned = strdup(text);
  if (owned == NULL) {
    ESP_LOGE(TELEGRAM_TAG, "No memory for text message");
    return;
  }
  const bus_msg_t msg = {
      .action = ACTION_SND_MSG,
      .value.valT = owned,
  };
  bus_send(msg, BUS_DEFAULT_TTL);
}

void telegram_send_jpg_async(const uint8_t* jpg, const size_t len,
                             const char* caption)
{
  bus_img img;

  img.jpg = heap_caps_malloc(len, MALLOC_CAP_SPIRAM);
  if (!img.jpg) {
    ESP_LOGI(TELEGRAM_TAG, "No memory for jpg");

    return;
  }
  memcpy(img.jpg, jpg, len);
  img.len = len;
  snprintf(
      img.caption, sizeof(img.caption), "%s", caption ? caption : APP_NAME);

  const bus_msg_t msg = {
      .action = ACTION_SND_JPG,
      .value.valImg = img,
  };
  bus_send(msg, BUS_DEFAULT_TTL);
}

esp_err_t telegram_send_photo(const uint8_t* jpg, size_t len,
                              const char* caption)
{
  char url[256];
  static char response[512];
  snprintf(
      url, sizeof(url), "https://api.telegram.org/bot%s/sendPhoto", TG_TOKEN);
  const char* boundary = "----ESPBoundary7MA4YWxkTrZu0gW";
  char part_hdr[512];
  const int hdr_len = snprintf(part_hdr, sizeof(part_hdr),
                                 "--%s\r\n"
                                 "Content-Disposition: form-data; name=\"chat_id\"\r\n\r\n"
                                 "%s\r\n"
                                 "--%s\r\n"
                                 "Content-Disposition: form-data; name=\"caption\"\r\n\r\n"
                                 "%s\r\n"
                                 "--%s\r\n"
                                 "Content-Disposition: form-data; name=\"photo\"; filename=\"cam.jpg\"\r\n"
                                 "Content-Type: image/jpeg\r\n\r\n",
                                 boundary, TG_CHAT_ID,
                                 boundary, caption ? caption : APP_NAME,
                                 boundary);

  char part_end[64];
  const int end_len =
      snprintf(part_end, sizeof(part_end), "\r\n--%s--\r\n", boundary);
  const int total = hdr_len + (int)len + end_len;

  uint8_t* body = heap_caps_malloc(total, MALLOC_CAP_SPIRAM);
  if (!body) {
    ESP_LOGE(TELEGRAM_TAG, "No memory for request body");
    return ESP_ERR_NO_MEM;
  }
  memcpy(body, part_hdr, hdr_len);
  memcpy(body + hdr_len, jpg, len);
  memcpy(body + hdr_len + len, part_end, end_len);

  char ct[128];
  snprintf(ct, sizeof(ct), "multipart/form-data; boundary=%s", boundary);
  const esp_err_t ret =
      http_post_req(url, (char*)body, total, ct, response, sizeof(response));
  free(body);
  ESP_LOGI(TELEGRAM_TAG, "Response: %s", response);

  return ret;
}

esp_err_t telegram_send_video_file(const char* path, const char* caption)
{
  char url[256];
  snprintf(
      url, sizeof(url), "https://api.telegram.org/bot%s/sendVideo", TG_TOKEN);

  const char* boundary = "----ESPBoundary7MA4YWxkTrZu0gW";
  const char* fname = strrchr(path, '/');
  fname = fname ? fname + 1 : path;

  char preamble[512];
  const int pre_len = snprintf(preamble, sizeof(preamble),
                                 "--%s\r\n"
                                 "Content-Disposition: form-data; name=\"chat_id\"\r\n\r\n"
                                 "%s\r\n"
                                 "--%s\r\n"
                                 "Content-Disposition: form-data; name=\"caption\"\r\n\r\n"
                                 "%s\r\n"
                                 "--%s\r\n"
                                 "Content-Disposition: form-data; name=\"video\"; filename=\"%s\"\r\n"
                                 "Content-Type: video/x-msvideo\r\n\r\n",
                                 boundary, TG_CHAT_ID,
                                 boundary, caption ? caption : APP_NAME,
                                 boundary, fname);

  char trailer[64];
  const int trl_len =
      snprintf(trailer, sizeof(trailer), "\r\n--%s--\r\n", boundary);

  if (pre_len < 0 || pre_len >= (int)sizeof(preamble) || trl_len < 0
      || trl_len >= (int)sizeof(trailer))
  {
    ESP_LOGE(TELEGRAM_TAG, "multipart header too long, aborting sendVideo");
    return ESP_FAIL;
  }

  char ct[128];
  snprintf(ct, sizeof(ct), "multipart/form-data; boundary=%s", boundary);

  static char response[512];
  const esp_err_t ret = http_post_multipart_file(url,
                                                 ct,
                                                 preamble,
                                                 pre_len,
                                                 path,
                                                 trailer,
                                                 trl_len,
                                                 response,
                                                 sizeof(response));
  ESP_LOGI(TELEGRAM_TAG, "sendVideo response: %s", response);
  return ret;
}
