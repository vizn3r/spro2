#include "http.h"
#include "cJSON.h"
#include "esp_err.h"
#include "esp_http_client.h"
#include "esp_log.h"
#include <stdlib.h>

cJSON *http_get_json(const char *url) {
  esp_http_client_config_t conf = {.url = url, .timeout_ms = 5000};

  esp_http_client_handle_t client = esp_http_client_init(&conf);
  if (client == NULL) {
    ESP_LOGE("HTTP", "HTTP client inti failed");
    return NULL;
  }

  esp_err_t ret = esp_http_client_perform(client);
  if (ret != ESP_OK) {
    ESP_LOGE("HTTP", "HTTP GET failed: %s", esp_err_to_name(ret));
    return NULL;
  }

  int status_code = esp_http_client_get_status_code(client);
  if (status_code != 200) {
    ESP_LOGE("HTTP", "HTTP resp code: %d", status_code);
    esp_http_client_cleanup(client);
    return NULL;
  }

  int clen = esp_http_client_get_content_length(client);
  if (clen <= 0 || clen > 2048)
    clen = 2048;

  char *buff = malloc(clen + 1);
  if (!buff) {
    ESP_LOGE("HTTP", "Buffer alloc failed");
    esp_http_client_cleanup(client);
    return NULL;
  }

  int read_len = esp_http_client_read_response(client, buff, clen);
  buff[read_len] = '\0';

  cJSON *json = cJSON_Parse(buff);
  if (!json) {
    ESP_LOGE("HTTP", "Json parsing failed");
  }

  free(buff);
  esp_http_client_cleanup(client);

  return json;
}
