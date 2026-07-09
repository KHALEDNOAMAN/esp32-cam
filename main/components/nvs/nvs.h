//
// Created by admin on 19.04.2026.
//

#ifndef SP_CAM_NVS_H
#define SP_CAM_NVS_H
#include "esp_err.h"

esp_err_t nvs_init(void);
void nvs_save_pin_state(const uint8_t pin, const int state);
int nvs_get_pin_state(const uint8_t pin, const int default_val);
void nvs_save_wifi_credentials(const char* ssid, const char* password);
bool nvs_get_wifi_credentials(char* ssid, size_t ssid_len, char* password, size_t password_len);
#endif //SP_CAM_NVS_H
