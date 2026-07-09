#include "http_server.h"
#include "components/camera/camera.h"
#include "components/config/config.h"
#include "components/http_server/actions/actions.h"
#include "esp_http_server.h"
#include "esp_log.h"
#include <stdio.h>

static const char* HTTP_TAG = "HTTP";
static httpd_handle_t s_httpd = NULL;

static const char* method_str(const httpd_method_t m)
{
  switch (m) {
    case HTTP_GET:
      return "GET ";
    case HTTP_POST:
      return "POST";
    default:
      return "??? ";
  }
}

esp_err_t http_server_start(char* host)
{
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();
  config.server_port = HTTP_SERVER_PORT;
  config.ctrl_port = 32769;
  config.max_uri_handlers = 11;
  config.max_open_sockets = 4;
  config.lru_purge_enable = true;
  config.recv_wait_timeout = 10;
  config.send_wait_timeout = 30;
  config.stack_size = 8192 * 2;
  config.task_priority = 5;
  config.uri_match_fn = httpd_uri_match_wildcard;

  ESP_LOGI(HTTP_TAG, "Starting HTTP server on port %d", config.server_port);
  if (httpd_start(&s_httpd, &config) != ESP_OK) {
    ESP_LOGE(HTTP_TAG, "Failed to start HTTP server");

    return ESP_FAIL;
  }

  const httpd_uri_t uris[] = {
      {.uri = "/", .method = HTTP_GET, .handler = index_handler},
      {.uri = "/stream", .method = HTTP_GET, .handler = stream_handler},
      {.uri = "/capture", .method = HTTP_GET, .handler = capture_handler},
      {.uri = "/save", .method = HTTP_GET, .handler = save_image_handler},
      {.uri = "/record", .method = HTTP_GET, .handler = record_handler},
      {.uri = "/sse", .method = HTTP_GET, .handler = sse_handler},
      {.uri = "/control", .method = HTTP_GET, .handler = cmd_camera_action},
      {.uri = "/status", .method = HTTP_GET, .handler = status_camera_action},
      {.uri = "/ws",
       .method = HTTP_GET,
       .handler = ws_handler,
       .is_websocket = true,
       .ws_post_handshake_cb = ws_post_handshake},
  };
  for (int i = 0; i < sizeof(uris) / sizeof(uris[0]); i++) {
    httpd_register_uri_handler(s_httpd, &uris[i]);
    ESP_LOGI(HTTP_TAG,
             "%s http://%s.local:%d%s",
             method_str(uris[i].method),
             host,
             HTTP_SERVER_PORT,
             uris[i].uri);
  }

  return ESP_OK;
}

void http_server_stop(void)
{
  if (s_httpd) {
    httpd_stop(s_httpd);
    s_httpd = NULL;
  }
}
