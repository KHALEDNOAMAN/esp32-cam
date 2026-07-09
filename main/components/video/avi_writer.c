//
// MJPEG-in-AVI container writer.
//

#include "avi_writer.h"
#include "esp_log.h"
#include <esp_heap_caps.h>
#include <string.h>

static const char* AVI_TAG = "AVI";

static inline void w_u32(FILE* f, uint32_t v)
{
  fwrite(&v, 4, 1, f);
}

static inline void w_tag(FILE* f, const char* t)
{
  fwrite(t, 1, 4, f);
}

esp_err_t avi_open(avi_writer_t* w, const char* path, uint32_t width,
                   uint32_t height, uint32_t max_frames)
{
  memset(w, 0, sizeof(*w));
  w->idx = heap_caps_malloc((size_t)max_frames * 4 * sizeof(uint32_t),
                            MALLOC_CAP_SPIRAM);
  if (!w->idx) {
    ESP_LOGE(
        AVI_TAG, "No PSRAM for idx (%lu frames)", (unsigned long)max_frames);
    return ESP_ERR_NO_MEM;
  }
  w->idx_cap = max_frames;
  w->width = width;
  w->height = height;

  w->f = fopen(path, "wb");
  if (!w->f) {
    ESP_LOGE(AVI_TAG, "Cannot open %s", path);
    heap_caps_free(w->idx);
    w->idx = NULL;
    return ESP_FAIL;
  }
  FILE* f = w->f;

  // RIFF
  w_tag(f, "RIFF");
  w_u32(f, 0);  // [4] fileSize (patched)
  w_tag(f, "AVI ");
  // LIST hdrl
  w_tag(f, "LIST");
  w_u32(f, 192);
  w_tag(f, "hdrl");
  // avih (56-byte body)
  w_tag(f, "avih");
  w_u32(f, 56);
  w_u32(f, 0);  // [32] dwMicroSecPerFrame (patched)
  w_u32(f, 0);  // dwMaxBytesPerSec
  w_u32(f, 0);  // dwPaddingGranularity
  w_u32(f, 0x10);  // dwFlags = AVIF_HASINDEX
  w_u32(f, 0);  // [48] dwTotalFrames (patched)
  w_u32(f, 0);  // dwInitialFrames
  w_u32(f, 1);  // dwStreams
  w_u32(f, 0);  // dwSuggestedBufferSize
  w_u32(f, width);  // dwWidth
  w_u32(f, height);  // dwHeight
  w_u32(f, 0);
  w_u32(f, 0);
  w_u32(f, 0);
  w_u32(f, 0);  // dwReserved[4]
  // LIST strl
  w_tag(f, "LIST");
  w_u32(f, 116);
  w_tag(f, "strl");
  // strh (56-byte body)
  w_tag(f, "strh");
  w_u32(f, 56);
  w_tag(f, "vids");  // fccType
  w_tag(f, "MJPG");  // fccHandler
  w_u32(f, 0);  // dwFlags
  w_u32(f, 0);  // wPriority / wLanguage
  w_u32(f, 0);  // dwInitialFrames
  w_u32(f, 0);  // [128] dwScale (patched)
  w_u32(f, 0);  // [132] dwRate  (patched)
  w_u32(f, 0);  // dwStart
  w_u32(f, 0);  // [140] dwLength (patched)
  w_u32(f, 0);  // dwSuggestedBufferSize
  w_u32(f, 0);  // dwQuality
  w_u32(f, 0);  // dwSampleSize
  w_u32(f, 0);  // rcFrame left/top
  w_u32(f, (height << 16) | width);  // rcFrame right/bottom
  // strf (BITMAPINFOHEADER, 40-byte body)
  w_tag(f, "strf");
  w_u32(f, 40);
  w_u32(f, 40);  // biSize
  w_u32(f, width);  // biWidth
  w_u32(f, height);  // biHeight
  w_u32(f, (24 << 16) | 1);  // biPlanes=1, biBitCount=24
  w_tag(f, "MJPG");  // biCompression
  w_u32(f, width * height * 3);  // biSizeImage
  w_u32(f, 0);
  w_u32(f, 0);  // biX/YPelsPerMeter
  w_u32(f, 0);
  w_u32(f, 0);  // biClrUsed / biClrImportant
  // LIST movi
  w_tag(f, "LIST");
  w_u32(f, 0);
  w_tag(f, "movi");  // [216] moviSize (patched)

  w->movi_size = 4;  // includes the 'movi' fourcc
  w->frame_count = 0;
  return ESP_OK;
}

esp_err_t avi_write_frame(avi_writer_t* w, const uint8_t* jpg, size_t len)
{
  if (!w->f) {
    return ESP_FAIL;
  }
  if (w->frame_count >= w->idx_cap) {
    return ESP_ERR_NO_MEM;
  }
  FILE* f = w->f;

  const uint32_t chunk_offset =
      w->movi_size;  // relative to 'movi' fourcc (first frame = 4)
  w_tag(f, "00dc");
  w_u32(f, (uint32_t)len);
  if (fwrite(jpg, 1, len, f) != len) {
    return ESP_FAIL;
  }
  uint32_t total = 8 + (uint32_t)len;
  if (len & 1) {
    const uint8_t pad = 0;
    fwrite(&pad, 1, 1, f);
    total++;
  }

  uint32_t* e = &w->idx[w->frame_count * 4];
  memcpy(&e[0], "00dc", 4);
  e[1] = 0x10;  // AVIIF_KEYFRAME
  e[2] = chunk_offset;
  e[3] = (uint32_t)len;

  w->movi_size += total;
  w->frame_count++;
  return ESP_OK;
}

esp_err_t avi_finalize(avi_writer_t* w, float measured_fps)
{
  if (!w->f) {
    return ESP_FAIL;
  }
  FILE* f = w->f;
  if (measured_fps < 1.0f) {
    measured_fps = 1.0f;
  }

  // idx1
  w_tag(f, "idx1");
  w_u32(f, w->frame_count * 16);
  fwrite(w->idx, 16, w->frame_count, f);

  const long total = ftell(f);
  const uint32_t usec = (uint32_t)(1000000.0f / measured_fps + 0.5f);
  const uint32_t scale = 1000;
  const uint32_t rate = (uint32_t)(measured_fps * 1000.0f + 0.5f);

  fseek(f, 32, SEEK_SET);
  w_u32(f, usec);
  fseek(f, 48, SEEK_SET);
  w_u32(f, w->frame_count);
  fseek(f, 128, SEEK_SET);
  w_u32(f, scale);
  fseek(f, 132, SEEK_SET);
  w_u32(f, rate);
  fseek(f, 140, SEEK_SET);
  w_u32(f, w->frame_count);
  fseek(f, 216, SEEK_SET);
  w_u32(f, w->movi_size);
  fseek(f, 4, SEEK_SET);
  w_u32(f, (uint32_t)(total - 8));

  if (ferror(f)) {
    ESP_LOGE(AVI_TAG, "write error while finalizing %s", "AVI");
    fclose(f);
    w->f = NULL;
    heap_caps_free(w->idx);
    w->idx = NULL;
    return ESP_FAIL;
  }

  fclose(f);
  w->f = NULL;
  heap_caps_free(w->idx);
  w->idx = NULL;
  ESP_LOGI(AVI_TAG,
           "finalized: %lu frames, %ld bytes",
           (unsigned long)w->frame_count,
           total);
  return ESP_OK;
}
