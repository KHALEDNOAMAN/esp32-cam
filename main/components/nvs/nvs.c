#include "nvs.h"
#include "esp_err.h"
#include "nvs_flash.h"

static nvs_handle_t s_nvs_handle = 0;
static bool s_handle_open = false;

esp_err_t nvs_init(void)
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }

    if (ret == ESP_OK) {
        ret = nvs_open("storage", NVS_READWRITE, &s_nvs_handle);
        if (ret == ESP_OK) {
            s_handle_open = true;
        }
    }

    return ret;
}

void nvs_save_pin_state(const uint8_t pin, const int state)
{
    if (!s_handle_open) {
        return;
    }
    char key[8];
    snprintf(key, sizeof(key), "p%d", pin);
    nvs_set_i32(s_nvs_handle, key, state);
    nvs_commit(s_nvs_handle);
}

int nvs_get_pin_state(const uint8_t pin, const int default_val)
{
    if (!s_handle_open) {
        return default_val;
    }
    char key[8];
    int32_t value = default_val;
    snprintf(key, sizeof(key), "p%d", pin);
    nvs_get_i32(s_nvs_handle, key, &value);
    return value;
}

void nvs_save_wifi_credentials(const char* ssid, const char* password)
{
    if (!s_handle_open) {
        return;
    }
    nvs_set_str(s_nvs_handle, "wifi_ssid", ssid);
    nvs_set_str(s_nvs_handle, "wifi_pass", password);
    nvs_commit(s_nvs_handle);
}

bool nvs_get_wifi_credentials(char* ssid, size_t ssid_len, char* password,
                              size_t password_len)
{
    if (!s_handle_open) {
        return false;
    }

    const esp_err_t err_ssid =
        nvs_get_str(s_nvs_handle, "wifi_ssid", ssid, &ssid_len);
    const esp_err_t err_pass =
        nvs_get_str(s_nvs_handle, "wifi_pass", password, &password_len);

    return (err_ssid == ESP_OK && err_pass == ESP_OK);
}
