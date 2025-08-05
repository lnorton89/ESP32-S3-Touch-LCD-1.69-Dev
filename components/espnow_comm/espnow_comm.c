#include <string.h>
#include <stdio.h>
#include "esp_log.h"
#include "esp_now.h"
#include "esp_mac.h"
#include "esp_wifi.h"
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "espnow_comm.h"

static const char *TAG = "ESP_NOW_COMM";
typedef struct {
    uint8_t mac_addr[ESP_NOW_ETH_ALEN];
    char message[250];
} esp_now_message_t;

static uint8_t s_peer_mac[ESP_NOW_ETH_ALEN] = {0xe4, 0xb3, 0x23, 0xb4, 0x93, 0x18};

static void wifi_init(void) {
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());
}

#if defined(CONFIG_ESP_NOW_SENDER)

static void esp_now_send_cb(const uint8_t *mac_addr, esp_now_send_status_t status) {
    ESP_LOGI(TAG, "Send to " MACSTR ", status: %s", MAC2STR(mac_addr),
             status == ESP_NOW_SEND_SUCCESS ? "Success" : "Failure");
}

static void sender_task(void *arg) {
    uint8_t my_mac[ESP_NOW_ETH_ALEN];
    ESP_ERROR_CHECK(esp_wifi_get_mac(WIFI_IF_STA, my_mac));
    esp_now_message_t message;
    memcpy(message.mac_addr, my_mac, ESP_NOW_ETH_ALEN);
    int count = 0;
    while (1) {
        snprintf(message.message, sizeof(message.message),
                 "Hello from sender! Count: %d", count++);
        ESP_LOGI(TAG, "Sending message: %s", message.message);
        esp_now_send(s_peer_mac, (uint8_t *)&message, sizeof(message));
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

void espnow_init_sender(void) {
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    wifi_init();
    uint8_t my_mac[ESP_NOW_ETH_ALEN];
    ESP_ERROR_CHECK(esp_wifi_get_mac(WIFI_IF_STA, my_mac));
    ESP_LOGI(TAG, "Sender MAC: " MACSTR, MAC2STR(my_mac));
    ESP_ERROR_CHECK(esp_now_init());
    ESP_ERROR_CHECK(esp_now_register_send_cb((esp_now_send_cb_t)esp_now_send_cb));
    esp_now_peer_info_t peer_info = {0};
    memcpy(peer_info.peer_addr, s_peer_mac, ESP_NOW_ETH_ALEN);
    peer_info.channel = 0;
    peer_info.encrypt = false;
    ESP_ERROR_CHECK(esp_now_add_peer(&peer_info));
    xTaskCreate(sender_task, "espnow_sender", 4096, NULL, 5, NULL);
}

#endif // CONFIG_ESP_NOW_SENDER

#if defined(CONFIG_ESP_NOW_RECEIVER)

static void esp_now_recv_cb(const uint8_t *mac_addr, const uint8_t *data, int len) {
    if (mac_addr && data && len >= sizeof(esp_now_message_t)) {
        esp_now_message_t *msg = (esp_now_message_t *)data;
        ESP_LOGI(TAG, "Received from " MACSTR ": %s", MAC2STR(mac_addr), msg->message);
    }
}

void espnow_init_receiver(void) {
    wifi_init();
    uint8_t my_mac[ESP_NOW_ETH_ALEN];
    ESP_ERROR_CHECK(esp_wifi_get_mac(WIFI_IF_STA, my_mac));
    ESP_LOGI(TAG, "Receiver MAC: " MACSTR, MAC2STR(my_mac));
    ESP_ERROR_CHECK(esp_now_init());
    ESP_ERROR_CHECK(esp_now_register_recv_cb(esp_now_recv_cb));
}

#endif // CONFIG_ESP_NOW_RECEIVER