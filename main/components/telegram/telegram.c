//
// Created by admin on 12.05.2026.
//

#include "telegram.h"

#include <esp_log.h>

#include "components/bus/bus.h"
#include "components/config/config.h"
#include "components/http_client/http_client.h"
static const char *TELEGRAM_TAG = "TELEGRAM";

esp_err_t telegram_send_text(const char *text) {
    static char response[512];
    char url[256];
    snprintf(url, sizeof(url),"https://api.telegram.org/bot%s/sendMessage", TG_TOKEN);

    char body[512];
    snprintf(body, sizeof(body), "{\"chat_id\":\"%s\",\"text\":\"%s\"}", TG_CHAT_ID, text);
    const esp_err_t ret = http_post_req(url, body, strlen(body), "application/json", response, sizeof(response));
    ESP_LOGI(TELEGRAM_TAG, "Response: %s", response);

    return ret;
}

void telegram_send_text_async(char *text) {
    const bus_msg_t msg = {
        .action = ACTION_SND_MSG,
        .value.valT = text,
    };
    bus_send(msg);
}

void telegram_send_jpg_async(const uint8_t *jpg, const size_t len, const char *caption) {
    bus_img img;

    img.jpg = malloc(len);
    if (!img.jpg) {
        ESP_LOGI(TELEGRAM_TAG, "No memory for jpg");

        return;
    }
    memcpy(img.jpg, jpg, len);
    img.len = len;
    snprintf(img.caption, sizeof(img.caption), "%s", caption ? caption : APP_NAME);

    const bus_msg_t msg = {
        .action = ACTION_SND_JPG,
        .value.valImg = img,
    };
    bus_send(msg);
}

esp_err_t telegram_send_photo(const uint8_t *jpg, size_t len, const char *caption) {
    char url[256];
    static char response[512];
    snprintf(url, sizeof(url),"https://api.telegram.org/bot%s/sendPhoto", TG_TOKEN);
    const char *boundary = "----ESPBoundary7MA4YWxkTrZu0gW";
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
    const int end_len = snprintf(part_end, sizeof(part_end),"\r\n--%s--\r\n", boundary);
    const int total = hdr_len + (int)len + end_len;

    uint8_t *body = malloc(total);
    if (!body) {
        ESP_LOGE(TELEGRAM_TAG, "No memory for request body");
        return ESP_ERR_NO_MEM;
    }
    memcpy(body, part_hdr, hdr_len);
    memcpy(body + hdr_len, jpg, len);
    memcpy(body + hdr_len + len, part_end, end_len);

    char ct[128];
    snprintf(ct, sizeof(ct),"multipart/form-data; boundary=%s", boundary);
    const esp_err_t ret = http_post_req(url, (char*)body, total, ct, response, sizeof(response));
    ESP_LOGI(TELEGRAM_TAG, "Response: %s", response);

    return ret;
}