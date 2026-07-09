//
// Created by admin on 10.05.2026.
//

#include "sdcard.h"
#include "driver/sdspi_host.h"
#include "driver/spi_common.h"
#include "esp_camera.h"
#include "esp_log.h"
#include "esp_vfs_fat.h"
#include <driver/sdmmc_host.h>
#include <stdio.h>

#include "components/config/config.h"

static const char *SD_TAG = "SDCARD";
static sdmmc_card_t *s_card = NULL;
static char s_file_path[64] = {0};
static uint32_t s_file_index = 0;

esp_err_t sdcard_init() {
    const esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = false,
        .max_files = SD_MAX_FILES,
        .allocation_unit_size = 16 * 1024
    };
    const sdmmc_host_t host = SDMMC_HOST_DEFAULT();
    sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();
    slot_config.width = 1;

    slot_config.clk = SD_PIN_CLK;
    slot_config.cmd = SD_PIN_CMD;
    slot_config.d0 = SD_PIN_D0;

    esp_err_t ret = esp_vfs_fat_sdmmc_mount(
        SD_MOUNT_POINT, &host, &slot_config, &mount_config, &s_card);

    if (ret != ESP_OK) {
        ESP_LOGE(SD_TAG, "Mount failed: %s", esp_err_to_name(ret));
        return ret;
    }
    ESP_LOGI(SD_TAG, "SD card mounted");

    return ESP_OK;
}

bool sdcard_is_mounted(void) {
    return s_card != NULL;
}

const char *sdcard_save_jpeg(const uint8_t *data, const size_t length) {
    if (!s_card) {
        ESP_LOGE(SD_TAG, "SD card not initialized");
        return NULL;
    }

    snprintf(s_file_path,
             sizeof(s_file_path),
             SD_MOUNT_POINT "/img%05lu.jpg",
             (unsigned long) s_file_index++);

    FILE *f = fopen(s_file_path, "wb");
    if (!f) {
        ESP_LOGE(SD_TAG, "Cannot open file: %s", s_file_path);
        return NULL;
    }

    const size_t written = fwrite(data, 1, length, f);
    fclose(f);

    if (written != length) {
        ESP_LOGE(SD_TAG,
                 "Write file (%s) error: %zu/%zu bytes",
                 s_file_path,
                 written,
                 length);
        return NULL;
    }

    ESP_LOGI(SD_TAG, "Saved %s (%zu bytes)", s_file_path, written);

    return s_file_path;
}

bool sd_append_text(const char *filename, const char *text) {
    char path[256];
    snprintf(path, sizeof(path), "%s/%s", SD_MOUNT_POINT, filename);
    FILE *f = fopen(path, "a");
    if (f == NULL) {
        return false;
    }
    fprintf(f, "%s\n", text);
    fclose(f);
    return true;
}

bool sd_append_f(const char *filename, const char *fmt, ...) {
    char path[256];
    snprintf(path, sizeof(path), "%s/%s", SD_MOUNT_POINT, filename);
    FILE *f = fopen(path, "a");
    if (f == NULL) {
        return false;
    }
    va_list args;
    va_start(args, fmt);
    vfprintf(f, fmt, args);
    va_end(args);
    fclose(f);
    return true;
}

void sdcard_de_init(void) {
    if (s_card) {
        const esp_err_t ret = esp_vfs_fat_sdcard_unmount(SD_MOUNT_POINT, s_card);
        s_card = NULL;
        if (ret != ESP_OK) {
            ESP_LOGE(SD_TAG, "SD unmount failed: %s", esp_err_to_name(ret));
            return;
        }
        ESP_LOGI(SD_TAG, "SD card unmounted");
    }
}
