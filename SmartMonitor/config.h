/*
 * 
 */
#pragma once

#define DEBUG_SERIAL
//#define USE_ETHERNET

const int BUZZZER_PIN = 5; // GIOP5 pin connected to piezo buzzer

#define EEPROM_MAX_SIZE    256
#define EEPROM_TEXT_SIZE   50
#define EEPROM_TEXT_OFFSET 6

#define MAX_DIALOG_TEXT_SIZE 1024

#define SM_VERSION "0.3"

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

enum HardwareType {
  noIO = 0,
  buzzer1Pulse = 1,
  buzzerPWM = 2,
  SoundI2S = 3,
  binarySensor = 4,
  numberSensor = 5,
  stateSensor = 6,
  tagScanner = 7,
  switchOutput = 8,
  lockOutput = 9,
  lightOutput = 10,
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
