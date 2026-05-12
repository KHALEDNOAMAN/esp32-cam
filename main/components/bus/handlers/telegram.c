//
// Created by admin on 17.04.2026.
//
#include "components/bus/bus.h"
#include "components/telegram/telegram.h"


static const char* IRH_TAG = "IR_H";
void telegram_send_msg(const bus_msg_t *msg) {
    telegram_send_text(msg->value.valT);
}

void telegram_send_jpg(const bus_msg_t *msg) {
    telegram_send_photo(msg->value.valImg.jpg, msg->value.valImg.len, msg->value.valImg.caption);
    free(msg->value.valImg.jpg);
}
