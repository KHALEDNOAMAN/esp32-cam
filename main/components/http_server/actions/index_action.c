//
// Created by admin on 10.05.2026.
//
#include "esp_http_server.h"

extern const uint8_t index_html_start[] asm("_binary_index_html_start");
extern const uint8_t index_html_end[] asm("_binary_index_html_end");

esp_err_t index_handler(httpd_req_t* req)
{
  httpd_resp_set_type(req, "text/html");
  return httpd_resp_send(
      req, (const char*)index_html_start, index_html_end - index_html_start);
}
