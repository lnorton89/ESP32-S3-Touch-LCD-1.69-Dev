#include "app_gui.h"
#include "esp_log.h"
#include "esp_lvgl_port.h"
#include "http_client.h" // Add this include to resolve the 'wait_for_ip' implicit declaration

static const char *TAG = "app_gui";

/**
 * @brief Button event callback function.
 * @param e LVGL event object.
 */
void app_button_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *btn = lv_event_get_target(e);

    if (code == LV_EVENT_PRESSED) {
        lv_obj_set_style_bg_color(btn, lv_color_hex(0x444444), LV_PART_MAIN | LV_STATE_PRESSED);
    } else if (code == LV_EVENT_RELEASED) {
        // Reset button color on release
        lv_obj_set_style_bg_color(btn, lv_color_hex(0x222222), LV_PART_MAIN);
    } else if (code == LV_EVENT_CLICKED) {
        ESP_LOGI(TAG, "Button was clicked");
        xTaskCreate(http_task, "http_task", 4096, NULL, 5, NULL);
    }
}

/**
 * @brief Creates the main display and its widgets.
 */
void app_main_display(void)
{
    // It is critical to lock the LVGL port before creating objects.
    lvgl_port_lock(-1);

    lv_obj_t *scr = lv_scr_act();
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, LV_PART_MAIN);

    // Create a label with centered text
    lv_obj_t *label = lv_label_create(scr);
    lv_obj_set_style_text_color(label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_label_set_text(label, "Hello ESP32-S3!");
    lv_obj_center(label);

    // Create a button
    lv_obj_t *btn = lv_btn_create(scr);
    lv_obj_set_size(btn, 120, 50);
    lv_obj_align(btn, LV_ALIGN_BOTTOM_MID, 0, -30);
    lv_obj_add_event_cb(btn, app_button_event_cb, LV_EVENT_ALL, NULL);

    lv_obj_t *btn_label = lv_label_create(btn);
    lv_label_set_text(btn_label, "Click Me");
    lv_obj_center(btn_label);

    // IMPORTANT: Unlock the LVGL port after all drawing and object creation is done.
    lvgl_port_unlock();
}
