#pragma once

#include <stdbool.h>

#define API_KEY ""
#define BASE_URL "http://api.weatherapi.com/v1"

#define CURR_WEATHER "/current.json"
#define FORECAST "/forecast.json"
#define TIMEZONE "/timezone.json"

// Refer to: https://www.weatherapi.com/docs/#apis-realtime
typedef struct {
  // Updates
  char *last_updated;
  int last_updated_epoch;
  // Temps
  float temp_c;
  float temp_f;
  float feelslike_c;
  float feelslike_f;
  float windchill_c;
  float windchill_f;
  float heatindex_c;
  float heatindex_f;
  float dewpoint_c;
  float dewpoint_f;

  // Weather
  char *condition_text;
  char *condition_icon;
  int condition_code;
  float wind_mph;
  float wind_kph;
  int wind_degree;
  char *wind_dir;
  float pressure_mb;
  float pressure_in;
  float precip_mm;
  float precip_in;
  float humidity;
  int cloud;
  bool is_day;
  float uv;
  float gust_mph;
  float gust_kph;

} curr_weather_t;

// Refer to: https://www.weatherapi.com/docs/#apis-forecast
typedef struct {
  // Temps
  float maxtemp_c;
  float maxtemp_f;
  float mintemp_c;
  float mintemp_f;
  float avgtemp_c;
  float avgtemp_f;

  // Weather
  float maxwind_mph;
  float maxwind_kph;
  float totalprecip_mm;
  float totalprecip_in;
  float totalsnow_cm;
  float avgvis_km;
  float avgvis_miles;
  int avghumidity;
  char *condition_text;
  char *condition_icon;
  int condition_code;
  float uv;
  bool daily_will_it_rain;
  bool daily_will_it_snow;
  int daily_chance_of_rain;
  int daily_chance_of_show;
} weather_day_t;

// Refer to: https://www.weatherapi.com/docs/#apis-forecast
typedef struct {
  // Times
  char *sunrise;
  char *sunset;
  char *moonrise;
  char *moonset;

  // Moon
  char *moon_phase;
  float moon_illumination;
  bool is_moon_up;
  bool is_sun_up;
} weather_astro_t;

// Refer to: https://www.weatherapi.com/docs/#intro-aqi
typedef struct {
  float co;
  float o3;
  float no2;
  float so2;
  float pm2_5;
  float pm10;
  int us_epa_index;
  int gb_defra_index;
} weather_aqi_t;

// Refer to: https://www.weatherapi.com/docs/#apis-forecast
typedef struct {
  // Time
  int time_epoch;
  char *time;

  // Temps
  float temp_c;
  float temp_f;
  float feelslike_c;
  float feelslike_f;
  float windchill_c;
  float windchill_f;
  float heatindex_c;
  float heatindex_f;
  float dewpoint_c;
  float dewpoint_f;

  // Weather
  char *condition_text;
  char *condition_icon;
  int condition_code;
  float wind_mph;
  float wind_kph;
  int wind_degree;
  char *wind_dir;
  float pressure_mb;
  float pressure_in;
  float precip_mm;
  float precip_in;
  float snow_cm;
  int humidity;
  int cloud;
  bool will_it_rain;
  bool will_it_snow;
  bool is_day;
  float vis_km;
  float vis_miles;
  int chance_of_rain;
  int chance_of_snow;
  float gust_mph;
  float gust_kph;
  float uv;
  float short_rad;
  float diff_rad;
  weather_aqi_t air_quality;

} weather_hour_t;

// Refer to: https://www.weatherapi.com/docs/#apis-forecast
typedef struct {
  char *date;
  int date_epoch;
  weather_day_t day;
  weather_astro_t astro;
  weather_aqi_t air_quality;

} forecast_t;
