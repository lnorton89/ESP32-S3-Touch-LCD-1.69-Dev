idf_component_register(SRCS "app_gui.c"
                    INCLUDE_DIRS "."
                    REQUIRES lvgl esp_lvgl_port http_client)