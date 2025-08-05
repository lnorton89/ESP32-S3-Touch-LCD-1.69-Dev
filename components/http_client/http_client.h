#ifndef HTTP_CLIENT_H
#define HTTP_CLIENT_H

#include "esp_err.h"
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

/**
 * @brief Performs an HTTP GET request to a specified URL.
 * @param url The URL for the GET request.
 * @return esp_err_t ESP_OK on success, or an error code on failure.
 */
esp_err_t http_client_get(const char *url);

/**
 * @brief Performs an HTTP POST request with a specified payload.
 * @param url The URL for the POST request.
 * @param post_data The JSON payload to send in the body of the request.
 * @return esp_err_t ESP_OK on success, or an error code on failure.
 */
esp_err_t http_client_post(const char *url, const char *post_data);

/**
 * @brief Starts the HTTP client task.
 * * This function creates a FreeRTOS task that handles all HTTP requests.
 */
void http_task(void *pvParameters);

#endif // HTTP_CLIENT_H
