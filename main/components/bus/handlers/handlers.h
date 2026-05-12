//
// Created by admin on 17.04.2026.
//

#ifndef SP_CAM_HANDLERS_H
#define SP_CAM_HANDLERS_H
#include "components/bus/bus.h"

void telegram_send_msg(const bus_msg_t *msg);
void telegram_send_jpg(const bus_msg_t *msg);
#endif //SP_CAM_HANDLERS_H
