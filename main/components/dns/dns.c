//
// Created by admin on 10.05.2026.
//

#include "dns.h"
#include <esp_err.h>
#include <esp_log.h>

#include "components/config/config.h"
#include "mdns.h"

static const char *DNS_TAG = "DNS";

esp_err_t dnsm_init(void) {
    const esp_err_t err = mdns_init();
    if (err != ESP_OK) {
        ESP_LOGE(DNS_TAG, "MDNS init failed: %d", err);

        return err;
    }
    ESP_ERROR_CHECK(mdns_hostname_set(HOST));
    ESP_ERROR_CHECK(mdns_instance_name_set("ESP CAM device"));

    return ESP_OK;
}

void dnsm_stop(void) {
    mdns_free();
}