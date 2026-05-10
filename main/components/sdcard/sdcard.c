//
// Created by admin on 10.05.2026.
//

#include "sdcard.h"
#include "config.h"
#include "esp_log.h"
#include "driver/spi_common.h"
#include "driver/sdspi_host.h"
#include "sdmmc_cmd.h"
#include "esp_vfs_fat.h"
#include <stdio.h>
#include <string.h>

static const char* SD_TAG = "SDCARD";
static sdmmc_card_t* s_card = NULL;
static char s_file_path[64] = {0};
static uint32_t s_file_index = 0;


esp_err_t sdcard_init(void) {
    ESP_LOGI(SD_TAG, "Mounting SD card (SPI)...");

    sdmmc_host_t host = SDSPI_HOST_DEFAULT();
    host.slot = SPI2_HOST;

    const spi_bus_config_t bus_cfg = {
        .mosi_io_num = SD_PIN_MOSI,
        .miso_io_num = SD_PIN_MISO,
        .sclk_io_num = SD_PIN_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 4096,
    };
    esp_err_t ret = spi_bus_initialize(host.slot, &bus_cfg, SDSPI_DEFAULT_DMA);
    if (ret != ESP_OK) {
        ESP_LOGE(SD_TAG, "SPI bus init failed: %s", esp_err_to_name(ret));
        return ret;
    }

    sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot_config.gpio_cs = SD_PIN_CS;
    slot_config.host_id = host.slot;

    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = false,
        .max_files = SD_MAX_FILES,
        .allocation_unit_size = 16 * 1024
    };

    ret = esp_vfs_fat_sdspi_mount(SD_MOUNT_POINT, &host, &slot_config, &mount_config, &s_card);
    if (ret != ESP_OK) {
        ESP_LOGE(SD_TAG, "SD mount failed: %s", esp_err_to_name(ret));
        if (ret == ESP_FAIL) {
            ESP_LOGE(SD_TAG, "Failed to mount FS. Format the card");
        }

        return ret;
    }

    sdmmc_card_print_info(stdout, s_card);
    ESP_LOGI(SD_TAG, "SD card mounted at: %s", SD_MOUNT_POINT);

    return ESP_OK;
}

const char* sdcard_save_jpeg(const uint8_t* data, const size_t length) {
    if (!s_card) {
        ESP_LOGE(SD_TAG, "SD card not mounted");
        return NULL;
    }

    snprintf(s_file_path, sizeof(s_file_path), SD_MOUNT_POINT "/img%05lu.jpg", (unsigned long) s_file_index++);
    FILE *f = fopen(s_file_path, "wb");
    if (!f) {
        ESP_LOGE(SD_TAG, "Cannot open file: %s", s_file_path);

        return NULL;
    }
    const size_t written = fwrite(data, 1, length, f);
    fclose(f);

    if (written != length) {
        ESP_LOGE(SD_TAG, "Write file (%s) error: %zu/%zu bytes", s_file_path, written, length);
        return NULL;
    }

    ESP_LOGI(SD_TAG, "Saved %s (%zu bytes)", s_file_path, written);

    return s_file_path;
}

void sdcard_deinit(void) {
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