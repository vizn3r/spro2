#include "api.h"
#include "cJSON.h"
#include "http.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
  WEATHER_RAINY,
  WEATHER_THUNDER,
  WEATHER_SUNNY,
  WEATHER_PART_CLOUDY,
  WEATHER_WINDY,
  WEATHER_CLOUDY,
  WEATHER_NIGHT,
  WEATHER_STORMY,
  WEATHER_SNOWY,
} weather_state_t;

weather_now_t CURRENT_WEATHER = {};

weather_now_t *parse_weather_data(cJSON *root) {
  if (!root)
    return NULL;

  weather_now_t *data = malloc(sizeof(weather_now_t));
  if (!data)
    return NULL;

  cJSON *location = cJSON_GetObjectItem(root, "location");
  if (location) {
    data->location.name =
        strdup(cJSON_GetObjectItem(location, "name")->valuestring);
    data->location.region =
        strdup(cJSON_GetObjectItem(location, "region")->valuestring);
    data->location.country =
        strdup(cJSON_GetObjectItem(location, "country")->valuestring);
    data->location.lat = cJSON_GetObjectItem(location, "lat")->valuedouble;
    data->location.lon = cJSON_GetObjectItem(location, "lon")->valuedouble;
    data->location.tz_id =
        strdup(cJSON_GetObjectItem(location, "tz_id")->valuestring);
    data->location.localtime_epoch =
        cJSON_GetObjectItem(location, "localtime_epoch")->valueint;
    data->location.localtime =
        strdup(cJSON_GetObjectItem(location, "localtime")->valuestring);
  }

  cJSON *current = cJSON_GetObjectItem(root, "current");
  if (current) {
    data->current.last_updated_epoch =
        cJSON_GetObjectItem(current, "last_updated_epoch")->valueint;
    data->current.last_updated =
        strdup(cJSON_GetObjectItem(current, "last_updated")->valuestring);
    data->current.temp_c = cJSON_GetObjectItem(current, "temp_c")->valuedouble;
    data->current.temp_f = cJSON_GetObjectItem(current, "temp_f")->valuedouble;
    data->current.is_day = cJSON_GetObjectItem(current, "is_day")->valueint;

    // Parse condition
    cJSON *condition = cJSON_GetObjectItem(current, "condition");
    if (condition) {
      data->current.condition.text =
          strdup(cJSON_GetObjectItem(condition, "text")->valuestring);
      data->current.condition.icon =
          strdup(cJSON_GetObjectItem(condition, "icon")->valuestring);
      data->current.condition.code =
          cJSON_GetObjectItem(condition, "code")->valueint;
    }

    data->current.wind_kph =
        cJSON_GetObjectItem(current, "wind_kph")->valuedouble;
    data->current.wind_degree =
        cJSON_GetObjectItem(current, "wind_degree")->valuedouble;
    data->current.wind_dir =
        strdup(cJSON_GetObjectItem(current, "wind_dir")->valuestring);
    data->current.pressure_mb =
        cJSON_GetObjectItem(current, "pressure_mb")->valuedouble;
    data->current.humidity = cJSON_GetObjectItem(current, "humidity")->valueint;
    data->current.cloud = cJSON_GetObjectItem(current, "cloud")->valueint;
    data->current.feelslike_c =
        cJSON_GetObjectItem(current, "feelslike_c")->valuedouble;
    data->current.vis_km = cJSON_GetObjectItem(current, "vis_km")->valuedouble;
    data->current.uv = cJSON_GetObjectItem(current, "uv")->valuedouble;
  }

  return data;
}

#include <string.h>

weather_state_t get_weather_state() {
  const char *text = CURRENT_WEATHER.current.condition.text;
  int is_day = CURRENT_WEATHER.current.is_day;
  float wind_kph = CURRENT_WEATHER.current.wind_kph;
  weather_state_t state;

  // Special case for code 1000 (Sunny/Clear)
  if (CURRENT_WEATHER.current.condition.code == 1000) {
    state = is_day ? WEATHER_SUNNY : WEATHER_NIGHT;
  } else {
    // Map based on condition text
    if (strstr(text, "Thundery outbreaks") != NULL ||
        strstr(text, "thunder") != NULL) {
      state = WEATHER_THUNDER;
    } else if (strstr(text, "Sunny") != NULL) {
      state = WEATHER_SUNNY;
    } else if (strstr(text, "Partly cloudy") != NULL) {
      state = WEATHER_PART_CLOUDY;
    } else if (strstr(text, "Cloudy") != NULL ||
               strstr(text, "Overcast") != NULL) {
      state = WEATHER_CLOUDY;
    } else if (strstr(text, "rain") != NULL ||
               strstr(text, "drizzle") != NULL) {
      state = WEATHER_RAINY;
    } else if (strstr(text, "snow") != NULL || strstr(text, "sleet") != NULL ||
               strstr(text, "blizzard") != NULL ||
               strstr(text, "ice pellets") != NULL) {
      state = WEATHER_SNOWY;
    } else if (strstr(text, "Mist") != NULL || strstr(text, "Fog") != NULL) {
      state = WEATHER_PART_CLOUDY;
    } else {
      // Default to partly cloudy if no other condition matches
      state = WEATHER_PART_CLOUDY;
    }
  }

  // Check for windy condition
  if (wind_kph >= 20.0) {
    if (state == WEATHER_SUNNY || state == WEATHER_NIGHT ||
        state == WEATHER_PART_CLOUDY || state == WEATHER_CLOUDY) {
      return WEATHER_WINDY;
    }
  }

  return state;
}

void get_weather_now(const char *location, const char *localization) {
  char url[256];
  snprintf(url, sizeof(url),
           "http://api.weatherapi.com/v1/current.json?key=%s&lang=%s&q=%s",
           API_KEY, localization, location);
  cJSON *data = http_get_json(url);
  if (!data) {
    return;
  }
  weather_now_t *weather = parse_weather_data(data);
  if (!weather) {
    cJSON_Delete(data);
    return;
  }

  free(CURRENT_WEATHER.location.name);
  free(CURRENT_WEATHER.location.region);
  free(CURRENT_WEATHER.location.country);
  free(CURRENT_WEATHER.location.tz_id);
  free(CURRENT_WEATHER.location.localtime);
  free(CURRENT_WEATHER.current.last_updated);
  free(CURRENT_WEATHER.current.condition.text);
  free(CURRENT_WEATHER.current.condition.icon);
  free(CURRENT_WEATHER.current.wind_dir);

  memcpy(&CURRENT_WEATHER, weather, sizeof(*weather));
  cJSON_Delete(data);
  free(weather);
}
