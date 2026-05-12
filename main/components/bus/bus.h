//
// Created by admin on 16.04.2026.
//

#ifndef SP_CAM_BUS_H
#define SP_CAM_BUS_H
#include "components/wifi/wifi.h"

typedef enum {
    ACTION_SND_MSG,
    ACTION_SND_JPG,
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
        int flag;
    } param;
    union {
        int valI;
        char* valT;
        uint8_t val8;
        float valF;
        bus_img valImg;
    } value;

} bus_msg_t;

typedef void (*bus_handler_t)(const bus_msg_t *msg);

void bus_init(bus_cfg_t *cfg);
void bus_register(bus_action_t action, bus_handler_t handler);
void bus_send(bus_msg_t msg);
void bus_send_isr(bus_msg_t msg);
#endif //SP_CAM_BUS_H
