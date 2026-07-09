#include "esp_http_server.h"
#include "esp_log.h"
#include <stdio.h>
#include <stdlib.h>

#include "components/config/config.h"
#include "components/http_server/http_helper/http_helper.h"
#include "components/video/video.h"

static const char* RECORD_ACTION_TAG = "RECORD_A";

esp_err_t record_handler(httpd_req_t* req)
{
  int sec = VIDEO_DEFAULT_SEC;

  const size_t qlen = httpd_req_get_url_query_len(req) + 1;
  if (qlen > 1) {
    char* buf = malloc(qlen);
    if (buf) {
      if (httpd_req_get_url_query_str(req, buf, qlen) == ESP_OK) {
        sec = parse_get_var(buf, "sec", VIDEO_DEFAULT_SEC);
      }
      free(buf);
    }
  }

  char path[80] = {0};
  const esp_err_t err = video_record_start(sec, path, sizeof(path));

  httpd_resp_set_type(req, "application/json");
  httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");

  if (err == ESP_OK) {
    char json[128];
    snprintf(json, sizeof(json), "{\"ok\":true,\"file\":\"%s\"}", path);
    httpd_resp_set_status(req, "202 Accepted");
    ESP_LOGI(RECORD_ACTION_TAG, "recording %s", path);
    return httpd_resp_sendstr(req, json);
  }
  if (err == ESP_ERR_INVALID_STATE) {
    httpd_resp_set_status(req, "409 Conflict");
    return httpd_resp_sendstr(req,
                              "{\"ok\":false,\"error\":\"already_recording\"}");
  }
  if (err == ESP_ERR_NOT_FOUND) {
    httpd_resp_set_status(req, "503 Service Unavailable");
    return httpd_resp_sendstr(req,
                              "{\"ok\":false,\"error\":\"sd_unavailable\"}");
  }
  // ESP_ERR_NO_MEM / ESP_FAIL: allocation or record-task spawn failure
  ESP_LOGE(RECORD_ACTION_TAG, "record start failed: %s", esp_err_to_name(err));
  httpd_resp_set_status(req, "500 Internal Server Error");
  return httpd_resp_sendstr(req,
                            "{\"ok\":false,\"error\":\"record_start_failed\"}");
}
