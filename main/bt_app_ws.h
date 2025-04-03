/*
 * SPDX-FileCopyrightText: 2021-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */

#ifndef __BT_APP_WS_H__
#define __BT_APP_WS_H__

#include <stdint.h>

// #define WS_TAG               "BT_APP_WS"
void websocket_app1_start(void);
void websocket_app1_stop(void);

void ws_Send_Stop(void);
void ws_Send_Start(void);

void wifi_sta_init(void);

void websocket_app_send(const char *buf, uint32_t sz);

void esp_print_tasks(void);

#endif /* __BT_APP_WS_H__*/
