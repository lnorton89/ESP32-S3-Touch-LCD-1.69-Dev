#include <string.h>
#include "esp_log.h"
#include "esp_http_client.h"
#include "cJSON.h"
#include "http_client.h"
#include "wifi_comm.h" // Add this include to resolve the 'wait_for_ip' implicit declaration

static const char *TAG = "HTTP_CLIENT";

// This is a buffer to hold the HTTP response data
static char *local_response_buffer = NULL;
// This holds the size of the buffer
static int local_response_buffer_size = 0;

/**
 * @brief The event handler for the HTTP client.
 * This function handles events from the HTTP client, such as receiving data.
 * The received data is appended to the local_response_buffer.
 */
esp_err_t _http_event_handler(esp_http_client_event_t *evt) {
    switch(evt->event_id) {
        case HTTP_EVENT_ERROR:
            ESP_LOGD(TAG, "HTTP_EVENT_ERROR");
            break;
        case HTTP_EVENT_ON_CONNECTED:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_CONNECTED");
            break;
        case HTTP_EVENT_HEADER_SENT:
            ESP_LOGD(TAG, "HTTP_EVENT_HEADER_SENT");
            break;
        case HTTP_EVENT_ON_HEADER:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_HEADER, key=%s, value=%s", evt->header_key, evt->header_value);
            break;
        case HTTP_EVENT_ON_DATA:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
            // If the buffer is not yet allocated or is too small, reallocate it
            if (local_response_buffer == NULL) {
                local_response_buffer = (char*)malloc(evt->data_len + 1);
                if (local_response_buffer == NULL) {
                    ESP_LOGE(TAG, "Failed to allocate memory for response buffer");
                    return ESP_FAIL;
                }
                local_response_buffer_size = evt->data_len;
                memcpy(local_response_buffer, evt->data, evt->data_len);
            } else {
                char *new_buffer = (char*)realloc(local_response_buffer, local_response_buffer_size + evt->data_len + 1);
                if (new_buffer == NULL) {
                    ESP_LOGE(TAG, "Failed to reallocate memory for response buffer");
                    free(local_response_buffer);
                    local_response_buffer = NULL;
                    return ESP_FAIL;
                }
                local_response_buffer = new_buffer;
                memcpy(local_response_buffer + local_response_buffer_size, evt->data, evt->data_len);
                local_response_buffer_size += evt->data_len;
            }
            local_response_buffer[local_response_buffer_size] = '\0'; // Null-terminate the string
            break;
        case HTTP_EVENT_ON_FINISH:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_FINISH");
            break;
        case HTTP_EVENT_DISCONNECTED:
            ESP_LOGD(TAG, "HTTP_EVENT_DISCONNECTED");
            break;
        default:
            break;
    }
    return ESP_OK;
}

/**
 * @brief Common function to perform an HTTP request.
 * @param url The URL to connect to.
 * @param method The HTTP method (GET or POST).
 * @param post_data The data to send with a POST request. Can be NULL for GET.
 * @return esp_err_t ESP_OK on success, or an error code on failure.
 */
static esp_err_t http_perform_request(const char *url, esp_http_client_method_t method, const char *post_data) {
    esp_http_client_config_t config = {
        .url = url,
        .event_handler = _http_event_handler,
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);
    if (!client) {
        ESP_LOGE(TAG, "Failed to initialize HTTP client");
        return ESP_FAIL;
    }
    
    // Set the request method
    esp_http_client_set_method(client, method);

    // If it's a POST request, set the data and content type
    if (method == HTTP_METHOD_POST && post_data != NULL) {
        esp_http_client_set_header(client, "Content-Type", "application/json");
        esp_http_client_set_post_field(client, post_data, strlen(post_data));
    }

    // Reset the buffer before each new request
    if (local_response_buffer) {
        free(local_response_buffer);
        local_response_buffer = NULL;
        local_response_buffer_size = 0;
    }

    // Perform the request
    esp_err_t err = esp_http_client_perform(client);
    
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "HTTP Request successful. Status = %d", esp_http_client_get_status_code(client));
        ESP_LOGI(TAG, "Response: %s", local_response_buffer);
        
        // Example of parsing the JSON response
        if (local_response_buffer) {
            cJSON *root = cJSON_Parse(local_response_buffer);
            if (root) {
                // You can process the JSON data here
                // For example, get a specific value:
                cJSON *field = cJSON_GetObjectItem(root, "my_field");
                if (field) {
                    ESP_LOGI(TAG, "my_field value: %s", cJSON_GetStringValue(field));
                }
                cJSON_Delete(root);
            }
        }

    } else {
        ESP_LOGE(TAG, "HTTP Request failed: %s", esp_err_to_name(err));
    }

    esp_http_client_cleanup(client);
    
    // Free the buffer after use
    if (local_response_buffer) {
        free(local_response_buffer);
        local_response_buffer = NULL;
        local_response_buffer_size = 0;
    }

    return err;
}


esp_err_t http_client_get(const char *url) {
    return http_perform_request(url, HTTP_METHOD_GET, NULL);
}

esp_err_t http_client_post(const char *url, const char *post_data) {
    return http_perform_request(url, HTTP_METHOD_POST, post_data);
}

/**
 * @brief A FreeRTOS task to perform an HTTP request.
 * This task waits for the Wi-Fi connection to be established and then
 * performs a simple HTTP GET request. This demonstrates how to use the
 * new http_client component without blocking the main application.
 * @param pvParameters Not used.
 */
void http_task(void *pvParameters) {
    // Wait for Wi-Fi to be connected before starting HTTP requests
    wait_for_ip();

    ESP_LOGI(TAG, "Wi-Fi is connected, starting HTTP client tasks.");

    // Example HTTP GET request
    ESP_LOGI(TAG, "Performing HTTP GET request...");
    http_client_get("http://httpbin.org/get");

    // Example HTTP POST request
    ESP_LOGI(TAG, "Performing HTTP POST request...");
    const char *post_json = "{\"key\": \"hello from esp32-s3\"}";
    http_client_post("http://httpbin.org/post", post_json);

    // The task can delete itself after it's done
    vTaskDelete(NULL);
}