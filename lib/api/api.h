#pragma once

#include <stdbool.h>

#define API_KEY ""
#define BASE_URL "http://api.weatherapi.com/v1"

#define CURR_WEATHER "/current.json"
#define FORECAST "/forecast.json"
#define TIMEZONE "/timezone.json"

typedef struct {
  char *name;
  char *region;
  char *country;
  double lat;
  double lon;
  char *tz_id;
  long localtime_epoch;
  char *localtime;
} api_location_t;

typedef struct {
  char *text;
  char *icon;
  int code;
} api_weather_condition_t;

// Refer to: https://www.weatherapi.com/docs/#apis-realtime
typedef struct {
  int last_updated_epoch;
  char *last_updated;
  double temp_c;
  double temp_f;
  bool is_day;
  api_weather_condition_t condition;
  double wind_mph;
  double wind_kph;
  int wind_degree;
  char *wind_dir;
  double pressure_mb;
  double pressure_in;
  double precip_mm;
  double precip_in;
  double humidity;
  int cloud;
  double feelslike_c;
  double feelslike_f;
  double windchill_c;
  double windchill_f;
  double heatindex_c;
  double heatindex_f;
  double dewpoint_c;
  double dewpoint_f;
  double vis_km;
  double vis_miles;
  double uv;
  double gust_mph;
  double gust_kph;
} curr_weather_t;

// Refer to: https://www.weatherapi.com/docs/#apis-forecast
typedef struct {
  // Temps
  double maxtemp_c;
  double maxtemp_f;
  double mintemp_c;
  double mintemp_f;
  double avgtemp_c;
  double avgtemp_f;

  // Weather
  double maxwind_mph;
  double maxwind_kph;
  double totalprecip_mm;
  double totalprecip_in;
  double totalsnow_cm;
  double avgvis_km;
  double avgvis_miles;
  int avghumidity;
  char *condition_text;
  char *condition_icon;
  int condition_code;
  double uv;
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
  double moon_illumination;
  bool is_moon_up;
  bool is_sun_up;
} weather_astro_t;

// Refer to: https://www.weatherapi.com/docs/#intro-aqi
typedef struct {
  double co;
  double o3;
  double no2;
  double so2;
  double pm2_5;
  double pm10;
  int us_epa_index;
  int gb_defra_index;
} weather_aqi_t;

// Refer to: https://www.weatherapi.com/docs/#apis-forecast
typedef struct {
  // Time
  int time_epoch;
  char *time;

  // Temps
  double temp_c;
  double temp_f;
  double feelslike_c;
  double feelslike_f;
  double windchill_c;
  double windchill_f;
  double heatindex_c;
  double heatindex_f;
  double dewpoint_c;
  double dewpoint_f;

  // Weather
  char *condition_text;
  char *condition_icon;
  int condition_code;
  double wind_mph;
  double wind_kph;
  int wind_degree;
  char *wind_dir;
  double pressure_mb;
  double pressure_in;
  double precip_mm;
  double precip_in;
  double snow_cm;
  int humidity;
  int cloud;
  bool will_it_rain;
  bool will_it_snow;
  bool is_day;
  double vis_km;
  double vis_miles;
  int chance_of_rain;
  int chance_of_snow;
  double gust_mph;
  double gust_kph;
  double uv;
  double short_rad;
  double diff_rad;
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

typedef struct {
  api_location_t location;
  curr_weather_t current;
} weather_now_t;
