#include "http.h"
#include "cJSON.h"
#include "esp_err.h"
#include "esp_http_client.h"
#include "esp_log.h"
#include <stdlib.h>
#include <string.h>

cJSON *http_get_json(const char *url) {
  esp_http_client_config_t conf = {
      .url = url,
      .timeout_ms = 5000,
      .disable_auto_redirect = false,
  };

  esp_http_client_handle_t client = esp_http_client_init(&conf);
  if (client == NULL) {
    ESP_LOGE("HTTP", "HTTP client init failed");
    return NULL;
  }

  // Open connection
  esp_err_t err = esp_http_client_open(client, 0);
  if (err != ESP_OK) {
    ESP_LOGE("HTTP", "HTTP open failed: %s", esp_err_to_name(err));
    esp_http_client_cleanup(client);
    return NULL;
  }

  // Fetch headers
  int status_code = esp_http_client_fetch_headers(client);
  if (status_code != 200) {
    ESP_LOGE("HTTP", "HTTP status code: %d", status_code);
    esp_http_client_cleanup(client);
    return NULL;
  }

  // Determine content length (may be -1 for chunked encoding)
  int content_len = esp_http_client_get_content_length(client);
  int buffer_size =
      (content_len > 0 && content_len <= 4096) ? content_len : 4096;

  char *buffer = malloc(buffer_size + 1); // +1 for null terminator
  if (!buffer) {
    ESP_LOGE("HTTP", "Failed to allocate buffer");
    esp_http_client_cleanup(client);
    return NULL;
  }

  int total_read = 0;
  int read_len;
  while (total_read < buffer_size) {
    read_len = esp_http_client_read(client, buffer + total_read,
                                    buffer_size - total_read);
    if (read_len <= 0) {
      break; // Error or end of data
    }
    total_read += read_len;
  }

  if (read_len < 0) {
    ESP_LOGE("HTTP", "Read error: %s", esp_err_to_name(read_len));
    free(buffer);
    esp_http_client_cleanup(client);
    return NULL;
  }

  buffer[total_read] = '\0'; // Null-terminate the string

  // Parse JSON
  cJSON *json = cJSON_Parse(buffer);
  if (!json) {
    const char *error_ptr = cJSON_GetErrorPtr();
    if (error_ptr != NULL) {
      ESP_LOGE("HTTP", "JSON parse error before: %s", error_ptr);
    }
  }

  free(buffer);
  esp_http_client_cleanup(client);
  return json;
}
