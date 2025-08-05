#ifndef APP_GUI_H
#define APP_GUI_H

#define LV_CONF_INCLUDE_SIMPLE
#include "lvgl.h"

void app_main_display(void);
void app_button_event_cb(lv_event_t *e);

#endif // APP_GUI_H