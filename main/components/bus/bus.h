//
// Created by admin on 16.04.2026.
//

#ifndef ESP_LORA_BUS_H
#define ESP_LORA_BUS_H
#include "freertos/FreeRTOS.h"
#include <freertos/task.h>
#include <freertos/timers.h>

typedef enum {
    ACTION_SND_MSG,
    ACTION_SND_JPG,
    ACTION_MOTION,
    ACTION_SND_LORA_MSG,
} bus_action_t;

typedef struct {
    int count;
    int delay;
} bus_cfg_t;

typedef struct {
    uint8_t *jpg;
    size_t   len;
    char     caption[128];
} bus_img;

typedef struct {
    bus_action_t action;
    union {
        int flagI;
        uint8_t flag8;
    } param;
    union {
        int valI;
        char* valT;
        uint8_t val8;
        float valF;
        bus_img valImg;
    } value;

} bus_msg_t;

typedef struct {
    bus_msg_t* msg;
    TickType_t enqueueTick;
    uint32_t ttlMs;
} bus_entry_t;

typedef void (*bus_handler_t)(const bus_msg_t *msg);

void bus_init(const bus_cfg_t *cfg);
void bus_register(bus_action_t action, bus_handler_t handler);
void bus_send(bus_msg_t msg, const uint32_t ttlMs);
void bus_send_isr(bus_msg_t msg, const uint32_t ttlMs);
#endif //ESP_LORA_BUS_H
