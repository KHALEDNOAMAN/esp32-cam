//
// MJPEG-in-AVI container writer.
//

#ifndef ESP_LORA_AVI_WRITER_H
#define ESP_LORA_AVI_WRITER_H

#include "esp_err.h"
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

typedef struct
{
  FILE* f;
  uint32_t frame_count;
  uint32_t movi_size;  // running LIST 'movi' size: 4 ('movi') + all chunk bytes
  uint32_t width;
  uint32_t height;
  uint32_t*
      idx;  // idx1 accumulator, 4 dwords per frame (ckid, flags, offset, size)
  uint32_t idx_cap;  // max frames the idx buffer can hold
} avi_writer_t;

// Open file and write the AVI/hdrl header with placeholder values (patched in
// finalize).
esp_err_t avi_open(avi_writer_t* w, const char* path, uint32_t width,
                   uint32_t height, uint32_t max_frames);

// Append one JPEG frame as a '00dc' chunk. Returns ESP_ERR_NO_MEM when idx cap
// is hit.
esp_err_t avi_write_frame(avi_writer_t* w, const uint8_t* jpg, size_t len);

// Append idx1, patch header fields (frame count, fps, sizes), close, free idx
// buffer.
esp_err_t avi_finalize(avi_writer_t* w, float measured_fps);

#endif  // ESP_LORA_AVI_WRITER_H
