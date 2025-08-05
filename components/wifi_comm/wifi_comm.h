#ifndef WIFI_COMM_H
#define WIFI_COMM_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define WIFI_SSID ""
#define WIFI_PASS ""

void wifi_init_sta(void);
void wait_for_ip(void);
void wifi_task(void *pvParameters);

#endif // WIFI_COMM_H