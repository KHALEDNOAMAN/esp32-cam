#include "http_client.h"
#include "esp_crt_bundle.h"
#include "esp_log.h"
#include <esp_http_client.h>
#include <stdio.h>
#include <stdlib.h>

static const char* HC_TAG = "HTTP_CLIENT";

static esp_err_t http_response_handler(esp_http_client_event_t* evt)
{
  http_response_t* resp = (http_response_t*)evt->user_data;

  switch (evt->event_id) {
    case HTTP_EVENT_ON_DATA:
      if (!esp_http_client_is_chunked_response(evt->client)) {
        int copy_len = evt->data_len;
        if (resp->length + copy_len >= resp->max_len) {
          copy_len = resp->max_len - resp->length - 1;
        }
        if (copy_len > 0) {
          memcpy(resp->buffer + resp->length, evt->data, copy_len);
          resp->length += copy_len;
          resp->buffer[resp->length] = '\0';
        }
      }
      break;
    default:
      break;
  }
  return ESP_OK;
}

esp_err_t http_get_req(const char* base_url, const char* query,
                       char* out_buffer, const int out_buffer_size)
{
  char url[256];
  if (query && strlen(query) > 0) {
    snprintf(url, sizeof(url), "%s?%s", base_url, query);
  } else {
    snprintf(url, sizeof(url), "%s", base_url);
  }
  http_response_t resp = {.buffer = out_buffer,
                          .max_len = out_buffer_size,
                          .length = 0,
                          .overflow = false};
  out_buffer[0] = '\0';

  const esp_http_client_config_t cfg = {
      .url = url,
      .method = HTTP_METHOD_GET,
      .event_handler = http_response_handler,
      .skip_cert_common_name_check = true,
      .use_global_ca_store = false,
      .cert_pem = NULL,
      .crt_bundle_attach = esp_crt_bundle_attach,
      .user_data = &resp,
      .timeout_ms = 15000,
      .buffer_size = 4096,
  };
  const esp_http_client_handle_t client = esp_http_client_init(&cfg);
  const esp_err_t err = esp_http_client_perform(client);
  if (err == ESP_OK) {
    ESP_LOGI(HC_TAG,
             "GET Status = %d, overflow=%d",
             esp_http_client_get_status_code(client),
             resp.overflow);
  } else {
    ESP_LOGE(HC_TAG, "GET request failed: %s", esp_err_to_name(err));
  }
  esp_http_client_cleanup(client);

  return err;
}

esp_err_t http_post_req(const char* url, const char* data, const int data_len,
                        const char* content_type, char* out_buffer,
                        const int out_buffer_size)
{
  http_response_t resp = {
      .buffer = out_buffer,
      .max_len = out_buffer_size,
      .length = 0,
      .overflow = false,
  };
  out_buffer[0] = '\0';

  const esp_http_client_config_t cfg = {
      .url = url,
      .method = HTTP_METHOD_POST,
      .skip_cert_common_name_check = true,
      .use_global_ca_store = false,
      .cert_pem = NULL,
      .crt_bundle_attach = esp_crt_bundle_attach,
      .timeout_ms = 20000,
      .buffer_size = 2048 * 4,
  };
  const esp_http_client_handle_t client = esp_http_client_init(&cfg);
  if (content_type) {
    esp_http_client_set_header(client, "Content-Type", content_type);
  }
  if (data) {
    ESP_LOGI(HC_TAG, "POST request: %d bytes", data_len);
    esp_http_client_set_post_field(client, data, data_len);
  }
  const esp_err_t err = esp_http_client_perform(client);
  if (err == ESP_OK) {
    ESP_LOGI(HC_TAG,
             "POST Status = %d, overflow=%d",
             esp_http_client_get_status_code(client),
             resp.overflow);
  } else {
    ESP_LOGE(HC_TAG, "POST failed: %s", esp_err_to_name(err));
  }
  esp_http_client_cleanup(client);
  return err;
}

esp_err_t http_post_multipart_file(const char* url, const char* content_type,
                                   const char* preamble, size_t preamble_len,
                                   const char* file_path, const char* trailer,
                                   size_t trailer_len, char* out_buffer,
                                   int out_buffer_size)
{
  esp_err_t err = ESP_FAIL;
  uint8_t* chunk = NULL;
  esp_http_client_handle_t client = NULL;

  FILE* fp = fopen(file_path, "rb");
  if (!fp) {
    ESP_LOGE(HC_TAG, "Cannot open %s for upload", file_path);
    return ESP_FAIL;
  }
  fseek(fp, 0, SEEK_END);
  const long f_size = ftell(fp);
  fseek(fp, 0, SEEK_SET);
  if (f_size < 0) {
    fclose(fp);
    return ESP_FAIL;
  }

  const int content_length = (int)preamble_len + (int)f_size + (int)trailer_len;

  const esp_http_client_config_t cfg = {
      .url = url,
      .method = HTTP_METHOD_POST,
      .skip_cert_common_name_check = true,
      .use_global_ca_store = false,
      .cert_pem = NULL,
      .crt_bundle_attach = esp_crt_bundle_attach,
      .timeout_ms = 60000,
      .buffer_size = 2048 * 4,
  };
  client = esp_http_client_init(&cfg);
  if (content_type) {
    esp_http_client_set_header(client, "Content-Type", content_type);
  }

  err = esp_http_client_open(client, content_length);
  if (err != ESP_OK) {
    ESP_LOGE(HC_TAG, "open failed: %s", esp_err_to_name(err));
    goto cleanup;
  }
  ESP_LOGI(
      HC_TAG, "multipart upload: %d bytes (file %ld)", content_length, f_size);

  if (esp_http_client_write(client, preamble, preamble_len) <= 0) {
    err = ESP_FAIL;
    goto cleanup;
  }

  chunk = malloc(4096);
  if (!chunk) {
    err = ESP_ERR_NO_MEM;
    goto cleanup;
  }
  size_t r;
  while ((r = fread(chunk, 1, 4096, fp)) > 0) {
    if (esp_http_client_write(client, (const char*)chunk, r) <= 0) {
      err = ESP_FAIL;
      goto cleanup;
    }
  }

  if (esp_http_client_write(client, trailer, trailer_len) <= 0) {
    err = ESP_FAIL;
    goto cleanup;
  }

  esp_http_client_fetch_headers(client);
  const int status = esp_http_client_get_status_code(client);
  if (out_buffer && out_buffer_size > 0) {
    int rd =
        esp_http_client_read_response(client, out_buffer, out_buffer_size - 1);
    if (rd < 0) {
      rd = 0;
    }
    out_buffer[rd] = '\0';
  }
  ESP_LOGI(HC_TAG, "multipart POST Status = %d", status);
  err = (status >= 200 && status < 300) ? ESP_OK : ESP_FAIL;

cleanup:
  if (chunk) {
    free(chunk);
  }
  if (client) {
    esp_http_client_cleanup(client);
  }
  fclose(fp);
  return err;
}
