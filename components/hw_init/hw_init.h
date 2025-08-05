#ifndef HW_INIT_H
#define HW_INIT_H

#include "esp_err.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lvgl_port.h"

#define EXAMPLE_LCD_H_RES (240)
#define EXAMPLE_LCD_V_RES (280)

esp_err_t hw_init_lcd(esp_lcd_panel_io_handle_t *io_handle, esp_lcd_panel_handle_t *panel_handle);
esp_err_t hw_init_touch(esp_lcd_panel_io_handle_t io_handle, esp_lcd_panel_handle_t panel_handle, lv_indev_drv_t *indev_drv);

#endif // HW_INIT_H