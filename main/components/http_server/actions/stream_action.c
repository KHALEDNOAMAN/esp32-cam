//
// Created by admin on 10.05.2026.
//
#include "components/config/config.h"
#include "img_converters.h"
#include "components/camera/camera.h"
#include "esp_log.h"
#include "esp_http_server.h"
#include <stdio.h>

static const char* STREAM_ACTION_TAG = "STREAM_ACTION";

// esp_err_t stream_handler(httpd_req_t *req) {
//     esp_err_t res = ESP_OK;
//     char part_buf[128];
//
//     res = httpd_resp_set_type(req, STREAM_CONTENT_TYPE);
//     if (res != ESP_OK) {
//         ESP_LOGE(STREAM_ACTION_TAG, "Set STREAM_CONTENT_TYPE failed");
//         return res;
//     }
//     httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
//     httpd_resp_set_hdr(req, "Cache-Control", "no-cache");
//
//     while (true) {
//         camera_fb_t *fb = esp_camera_fb_get();
//         if (!fb) {
//             vTaskDelay(2);
//             continue;
//         }
//
//         uint8_t *jpg = NULL;
//         size_t   jlen = 0;
//         bool     do_free = false;
//
//         if (fb->format == PIXFORMAT_JPEG) {
//             jpg = fb->buf;
//             jlen = fb->len;
//         } else {
//             do_free = frame2jpg(fb, JPEG_QUALITY, &jpg, &jlen);
//             if (!do_free) {
//                 jpg = NULL;
//             }
//         }
//         esp_camera_fb_return(fb);
//         if (!jpg) {
//             vTaskDelay(2);
//             continue;
//         }
//
//         size_t hlen = snprintf(part_buf, sizeof(part_buf),
//             "--" STREAM_BOUNDARY "\r\n"
//             "Content-Type: image/jpeg\r\nContent-Length: %zu\r\n\r\n", jlen);
//
//         res = httpd_resp_send_chunk(req, part_buf, hlen);
//         if (res == ESP_OK)
//             res = httpd_resp_send_chunk(req, (const char *)jpg, jlen);
//         if (res == ESP_OK)
//             res = httpd_resp_send_chunk(req, "\r\n", 2);
//
//         if (do_free) {
//             free(jpg);
//         }
//
//         if (res != ESP_OK) {
//             ESP_LOGI(STREAM_ACTION_TAG, "Stream client disconnected");
//             break;
//         }
//         taskYIELD();
//     }
//     vTaskPrioritySet(NULL, 5);
//     ESP_LOGI(STREAM_ACTION_TAG, "Stream ended");
//
//     return res;
// }

esp_err_t stream_handler(httpd_req_t *req) {
    camera_fb_t *fb = NULL;
    esp_err_t res = ESP_OK;
    char part_buf[128];

    res = httpd_resp_set_type(req, STREAM_CONTENT_TYPE);
    if (res != ESP_OK) {
        return res;
    }

    while (true) {
        fb = esp_camera_fb_get();
        if (!fb) {
            vTaskDelay(2);
            continue;
        }

        size_t h_len = snprintf(part_buf, sizeof(part_buf),
            "--" STREAM_BOUNDARY "\r\n"
                STREAM_PART,
                fb->len
        );

        res = httpd_resp_send_chunk(req, part_buf, h_len);
        if (res == ESP_OK) {
            res = httpd_resp_send_chunk(req, (const char*) fb->buf, fb->len);
        }
        if (res == ESP_OK) {
            res = httpd_resp_send_chunk(req, "\r\n", 2);
        }
        esp_camera_fb_return(fb);
        fb = NULL;

        if (res != ESP_OK) {
            ESP_LOGE(STREAM_ACTION_TAG, "Failed to send frame: %s", esp_err_to_name(res));
            break;
        }

        vTaskDelay(pdMS_TO_TICKS(40));
    }

    return res;
}