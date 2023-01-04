/*
 * 
 */
#pragma once

#define DEBUG_SERIAL
//#define USE_ETHERNET

const int BUZZZER_PIN = 18; // GIOP18 pin connected to piezo buzzer

#define EEPROM_MAX_SIZE    256
#define EEPROM_TEXT_SIZE   50
#define EEPROM_TEXT_OFFSET 6

#define MAX_DIALOG_TEXT_SIZE 1024

#define SM_VERSION "0.2"

#define JSON_MAX_SIZE 10000
#define JSON_PAYLOAD_SIZE 3000

#define MAX_ICON_SIZE 5000

#ifdef DEBUG_SERIAL
#define DEBUG(x) Serial.print(x)
#define DEBUGln(x) Serial.println(x)
#define DEBUGf(...) Serial.printf(__VA_ARGS__)
#else
#define DEBUG(x)
#define DEBUGln(x)
#define DEBUGf(x,y)
#endif 

enum PageType {
  normalPage = 0,
  keypadPage = 1,
  dialogPage = 2,
  // keep below last
  lastPageType,
};

enum BtnAction {
  None = 0,
  HAtrigger = 1,
  ChangePage = 2,
  Display = 3,
  Command = 4,
  // keep below last
  lastBtnAction,
};

// https://developers.home-assistant.io/docs/core/entity/
enum DataType {
  implicit = 0,
  alarm_state = 1,
  cover_state = 2,
  light_state = 3,
  lock_state = 4,
  switch_state = 5,
  weather_state = 6,
  apparent_power = 7,
  aqi = 8,
  atmospheric_pressure = 9,
  battery = 10,
  carbon_dioxide = 11,
  carbon_monoxide = 12,
  current = 13,
  distance = 14,
  duration = 15,
  energy = 16,
  frequency = 17,
  gas = 18,
  humidity = 19,
  illuminance = 20,
  irradiance = 21,
  moisture = 22,
  monetary = 23,
  nitrogen_dioxide = 24, 
  nitrogen_monoxide = 25, 
  nitrous_oxide = 26, 
  ozone = 27,
  pm1 = 28,
  pm25 = 29,
  pm10 = 30,
  power = 31,
  power_factor = 32,
  precipitation = 33,
  precipitation_intensity = 34,
  pressure = 35,
  reactive_power = 36,
  signal_strength = 37,
  sound_pressure = 38, 
  speed = 39,
  sulphur_dioxide = 40,
  temperature = 41,
  volatile_organic_compounds = 42,
  voltage = 43,
  volume = 44,
  water = 45,
  weight = 46,
  wind_speed = 47,
  elevation = 48,
  binary_state = 49,
  // keep below last
  lastDataType,
};

enum HardwareType {
  noIO = 0,
  buzzerTouch = 1,
  binarySensor = 2,
  numberSensor = 3,
  stateSensor = 4,
  tagScanner = 5,
  switchOutput = 6,
  lockOutput = 7,
  lightOutput = 8,
  // keep below last
  lastHardwareType,
};

enum ACParmType {
  none = -1,
  disarm = 0,
  armHome = 1,
  armAway = 2,
  armNight = 3,
  armVacation = 4,
  armCustom = 5,
};

enum displayMode {
  title_text = 1,
  data_text = 2,
  static_icon = 4,
  data_icon = 8,
};

struct mqttMsg {
  char* topic;
  char* payload;
  void create(const char* t, const char* p)
  {
    topic = new char[strlen(t) + 1];
    strcpy(topic, t);
    payload = new char[strlen(p) + 1];
    strcpy(payload, p);
  }
  void erase()
  {
    delete[] topic;
    delete[] payload;
  }
};
