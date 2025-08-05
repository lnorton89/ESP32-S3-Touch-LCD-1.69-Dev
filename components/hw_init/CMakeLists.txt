idf_component_register(SRCS "hw_init.c"
                    INCLUDE_DIRS "."
                    REQUIRES esp_lcd_touch_cst816s lvgl esp_lvgl_port)