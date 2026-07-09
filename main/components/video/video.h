//
// On-demand MJPEG/AVI recording to SD + Telegram upload.
//

#ifndef ESP_LORA_VIDEO_H
#define ESP_LORA_VIDEO_H

#include "esp_err.h"
#include <stdbool.h>
#include <stddef.h>

// Start a recording of `sec` seconds (clamped to VIDEO_MIN_SEC..VIDEO_MAX_SEC).
// On ESP_OK, out_path receives the target file path. Spawns a background task
// that records, finalizes the AVI, and uploads it to Telegram.
// Returns:
//   ESP_OK                -> accepted, recording started
//   ESP_ERR_NOT_FOUND     -> SD card not mounted
//   ESP_ERR_INVALID_STATE -> a recording is already in progress
//   ESP_ERR_NO_MEM / ESP_FAIL -> could not allocate/spawn
esp_err_t video_record_start(int sec, char* out_path, size_t out_sz);

bool video_is_recording(void);

#endif  // ESP_LORA_VIDEO_H
