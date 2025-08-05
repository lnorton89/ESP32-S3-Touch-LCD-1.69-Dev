#include "esp_err.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "hw_init.h"
#include "app_gui.h"
#include "wifi_comm.h"
#include "http_client.h"

#include "espnow_comm.h"

// Define a consistent log tag for the file
static const char *TAG = "app_main";

static esp_lcd_panel_io_handle_t lcd_io = NULL;
static esp_lcd_panel_handle_t lcd_panel = NULL;
static lv_indev_drv_t indev_drv_obj;

/**
 * @brief Initializes all application components
 * This function handles the setup for hardware, communication, and the GUI.
 */
static esp_err_t app_init(void) {
    ESP_LOGI(TAG, "Initializing application components...");

    // 1. Initialize the LCD and Touch
    ESP_ERROR_CHECK(hw_init_lcd(&lcd_io, &lcd_panel));
    ESP_ERROR_CHECK(hw_init_touch(lcd_io, lcd_panel, &indev_drv_obj));

    // 2. Initialize communication based on sdkconfig
    #if defined(CONFIG_ESP_NOW_SENDER)
        espnow_init_sender();
    #elif defined(CONFIG_ESP_NOW_RECEIVER)
        espnow_init_receiver();
    #else
        xTaskCreate(wifi_task, "wifi_task", 4096, NULL, 5, NULL);
    #endif

    // 3. Create the initial GUI
    app_main_display();

    ESP_LOGI(TAG, "Application initialization complete.");
    return ESP_OK;
}

void app_main(void)
{
    ESP_LOGI(TAG, "Application startup...");

    #ifdef MY_LV_CONF_INCLUDED
    ESP_LOGI(TAG, "SUCCESS: lv_conf.h is being included!");
    #else
        ESP_LOGE(TAG, "FAILURE: lv_conf.h is NOT being included!");
    #endif

    if (app_init() != ESP_OK) {
        ESP_LOGE(TAG, "Application failed to initialize. Restarting...");
        esp_restart();
    }
}