/*
  New esp32 library for ESP32S3 : https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
  Old : https://dl.espressif.com/dl/package_esp32_index.json

  Designed for : ESP32+TFT(custom), WT32-SC01, and future "SC01 Plus" when core 2.0 optimized and others library ready for.
  Core ESP32 : 1.0.6
  depending library:
    - LovyanGFX             (from Arduino IDE)
    - ArduinoJson           (from Arduino IDE)
    - HAintegration         (fork of home-assistant-integration)
    - ESPAsyncTCP           (from Arduino IDE)
    - ESPAsyncWebServer     (from Arduino IDE)

  test for PNG metatags : https://www.metadata2go.com/

*/
#include <rom/rtc.h>
#include "config.h"
#include <WiFi.h>
#include "FS.h"
#include "SPIFFS.h"
#include <HAintegration.h>
#include "display.h"
#include "deviceGPIO.hpp"
#include "network.hpp"

WiFiClient client;
HADevice device;
HAMqtt mqtt(client, device, 12); // 12 trigger MAX
HANumber* brightness;

extern LGFX lcd;

extern Xcontroler SMcontroler;

extern displayConfig_t displayConfig;
extern uint8_t displayOrientation;
extern bool mqttConnected;

uint32_t lastTime = 0;
bool oldSensor = false;
char* keypadPageNeeded = NULL;
bool isConfigLoaded = false;

void listDir(const char * dirname)
{
#ifdef DEBUG_SERIAL
  Serial.printf("Listing directory: %s\n", dirname);
  File root = SPIFFS.open(dirname);
  if (!root.isDirectory()) {
    Serial.println("No Dir");
  }
  File file = root.openNextFile();
  while (file) {
    Serial.print("  FILE: ");
    Serial.print(file.name());
    Serial.print("\tSIZE: ");
    Serial.println(file.size());
    file = root.openNextFile();
  }
#endif
}

void print_reset_reason(int reason)
{
  switch ( reason)
  {
    case 1 : DEBUGln("POWERON_RESET"); break;         /**<1,  Vbat power on reset*/
    case 3 : DEBUGln("SW_RESET"); break;              /**<3,  Software reset digital core*/
    case 4 : DEBUGln("OWDT_RESET"); break;            /**<4,  Legacy watch dog reset digital core*/
    case 5 : DEBUGln("DEEPSLEEP_RESET"); break;       /**<5,  Deep Sleep reset digital core*/
    case 6 : DEBUGln("SDIO_RESET"); break;            /**<6,  Reset by SLC module, reset digital core*/
    case 7 : DEBUGln("TG0WDT_SYS_RESET"); break;      /**<7,  Timer Group0 Watch dog reset digital core*/
    case 8 : DEBUGln("TG1WDT_SYS_RESET"); break;      /**<8,  Timer Group1 Watch dog reset digital core*/
    case 9 : DEBUGln("RTCWDT_SYS_RESET"); break;      /**<9,  RTC Watch dog Reset digital core*/
    case 10 : DEBUGln("INTRUSION_RESET"); break;      /**<10, Instrusion tested to reset CPU*/
    case 11 : DEBUGln("TGWDT_CPU_RESET"); break;      /**<11, Time Group reset CPU*/
    case 12 : DEBUGln("SW_CPU_RESET"); break;         /**<12, Software reset CPU*/
    case 13 : DEBUGln("RTCWDT_CPU_RESET"); break;     /**<13, RTC Watch dog Reset CPU*/
    case 14 : DEBUGln("EXT_CPU_RESET"); break;        /**<14, for APP CPU, reseted by PRO CPU*/
    case 15 : DEBUGln("RTCWDT_BROWN_OUT_RESET"); break; /**<15, Reset when the vdd voltage is not stable*/
    case 16 : DEBUGln("RTCWDT_RTC_RESET"); break;     /**<16, RTC Watch dog reset digital core and rtc module*/
    default : DEBUGln("NO_MEAN");
  }
}

hw_timer_t * timer = NULL;
volatile SemaphoreHandle_t timerSemaphore;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;
volatile bool isrTopSecond = false;

void IRAM_ATTR onTimer() {
  // Increment the counter and set the time of ISR
  portENTER_CRITICAL_ISR(&timerMux);
  isrTopSecond = true;
  portEXIT_CRITICAL_ISR(&timerMux);
  // Give a semaphore that we can check in the loop
  xSemaphoreGiveFromISR(timerSemaphore, NULL);
  // It is safe to use digitalRead/Write here if you want to toggle an output
}

void setupTimer()
{
  // Create semaphore to inform us when the timer has fired
  timerSemaphore = xSemaphoreCreateBinary();
  // Use 1st timer of 4 (counted from zero).
  // Set 80 divider for prescaler (see ESP32 Technical Reference Manual for more info).
  timer = timerBegin(0, 80, true);
  // Attach onTimer function to our timer.
  timerAttachInterrupt(timer, &onTimer, true);
  // Set alarm to call onTimer function every second (value in microseconds).
  // Repeat the alarm (third parameter)
  timerAlarmWrite(timer, 1000000, true);
  // Start an alarm
  timerAlarmEnable(timer);
}

void setup(void)
{
#ifdef DEBUG_SERIAL
  Serial.begin(115200);
  while (Serial.availableForWrite() == false) {
    delay(50);
  }
  print_reset_reason(rtc_get_reset_reason(0));
  print_reset_reason(rtc_get_reset_reason(1));
#endif
  DEBUGln("\nDebug start");

  if (!EEPROM.begin(EEPROM_MAX_SIZE))
  {
    DEBUGln("failed to initialise EEPROM");
  } else {
    /* uncomment to initialisze EEPROM with code variable, juste one run with that
        EEPROM.write(0, displayOrientation);
        EEPROM.writeChar(1, AP_ssid[8]);
        EEPROM.writeString(EEPROM_TEXT_OFFSET, Wifi_ssid);
        EEPROM.writeString(EEPROM_TEXT_OFFSET + (EEPROM_TEXT_SIZE * 1), Wifi_pass);
        EEPROM.writeString(EEPROM_TEXT_OFFSET + (EEPROM_TEXT_SIZE * 2), mqtt_host);
        EEPROM.writeString(EEPROM_TEXT_OFFSET + (EEPROM_TEXT_SIZE * 3), mqtt_user);
        EEPROM.writeString(EEPROM_TEXT_OFFSET + (EEPROM_TEXT_SIZE * 4), mqtt_pass);
        EEPROM.commit();
    */
    char code = EEPROM.readChar(1);
    if (code >= '0' && code <= '9')
    {
      displayOrientation = EEPROM.read(0) % 4;
      AP_ssid[8] = code;
      LumDevID[17] = code;
      strcpy(Wifi_ssid, EEPROM.readString(EEPROM_TEXT_OFFSET).c_str());
      strcpy(Wifi_pass, EEPROM.readString(EEPROM_TEXT_OFFSET + EEPROM_TEXT_SIZE * 1).c_str());
      strcpy(mqtt_host, EEPROM.readString(EEPROM_TEXT_OFFSET + EEPROM_TEXT_SIZE * 2).c_str());
      strcpy(mqtt_user, EEPROM.readString(EEPROM_TEXT_OFFSET + EEPROM_TEXT_SIZE * 3).c_str());
      strcpy(mqtt_pass, EEPROM.readString(EEPROM_TEXT_OFFSET + EEPROM_TEXT_SIZE * 4).c_str());
    }
    DEBUGln(displayOrientation);
    DEBUGln(AP_ssid);
    DEBUGln(Wifi_ssid);
    DEBUGln(Wifi_pass);
    DEBUGln(mqtt_host);
    DEBUGln(mqtt_user);
    DEBUGln(mqtt_pass);
  }
  SMcontroler.init();

  if (!SPIFFS.begin()) {
    DEBUGln("SPIFFS Mount failed");
  } else {
    DEBUGln("SPIFFS Mount succesfull");
    listDir("/");
    DEBUGf("SPIFFS used %d / total %d\n", SPIFFS.usedBytes(), SPIFFS.totalBytes());
  }
  SMcontroler.prepareLoading();
  configWifi();

  byte mac[6];
  WiFi.macAddress(mac);
  // HA
  device.setUniqueId(mac, sizeof(mac));
  device.setManufacturer("M&L");
#ifdef defined(SC01)
  device.setModel("SmartMonitor (WT32-SC01)");
#elif defined(SC01Plus)
  device.setModel("SmartMonitor (WT32-SC01 Plus)");
#else
  device.setModel("SmartMonitor (ESP32/TFT custom)");
#endif
  device.setName(AP_ssid);
  device.setSoftwareVersion(SM_VERSION);

  brightness = new HANumber(LumDevID);
  brightness->onCommand(onNumberCommand);
  brightness->setIcon("mdi:brightness-6");
  brightness->setName("Luminosité");
  brightness->setMin(10);
  brightness->setMax(255);
  brightness->setStep(5);
  brightness->setRetain(true);
  brightness->setState(200);

  mqtt.setDataPrefix("smartmonitor");
  mqtt.setBufferSize(2000);
  mqtt.onMessage(onMqttMessage);
  mqtt.onConnected(onMqttConnected);
  mqtt.onDisconnected(onMqttDisconnected);
  if (mqtt.begin(mqtt_host, mqtt_user, mqtt_pass))
  {
    DEBUGln("mqtt configured");
  } else {
    DEBUGln("mqtt error");
  }
  configWeb();
  int32_t x, y;
  if (lcd.getTouch(&x, &y))
  {
    lcd.println("No config loaded, wait for webserver action");
    while (lcd.getTouch(&x, &y));
    SMcontroler.setScratchMode();
  } else {
    LoadConfig();
    delay(2000);
    SMcontroler.drawBackground();
    SMcontroler.drawHeader("Attente serveur");
  }
  setupTimer();
}

void loop()
{
  bool isNewSec = false;
  if (xSemaphoreTake(timerSemaphore, 0) == pdTRUE)
  {
    portENTER_CRITICAL(&timerMux);
    isNewSec = isrTopSecond;
    isrTopSecond = false;
    portEXIT_CRITICAL(&timerMux);
  }
  if (isNewSec)
  {
    //DEBUGf("%d %d %d %d\n", ESP.getHeapSize(), ESP.getFreeHeap(), ESP.getMinFreeHeap(), ESP.getMaxAllocHeap());
  }
  if (isConfigLoaded)
  {
    mqtt.loop();
    SMcontroler.loop();
    for (int i = 0; i < nbGPIO; i++)
    {
      GPIOs[i].process(isNewSec);
    }
  } else
  {
    int32_t x, y;
    if (lcd.getTouch(&x, &y))
    {
      ESP.restart();
    }
  }
}

void onMqttMessage(const char* topic, const uint8_t* payload, uint16_t length) {
  char* pl = (char*)payload;
  pl[length] = 0;
  //DEBUGf("Mqtt Push -> %s = %s\n", topic, pl);
  SMcontroler.mqttMsgPush(topic, pl);
}

void onMqttConnected()
{
  mqttConnected = true;
  DEBUGf("MQTT connected\n");
  SMcontroler.drawHeader("--/--/---- --:--");
  mqtt.subscribe("smartmonitor/dateheure");
  mqtt.subscribe("smartmonitor/alarm_message");
  for (int p = 0; p < SMcontroler.getNbPages(); p++)
  {
    Xpage* page = SMcontroler.getPage(p);
    for (int i = 0; i < page->nbItems(); i++)
    {
      if (page->getItem(i)->isSuscribeMQTT())
      {
        DEBUGf("Suscribe %s\n", page->getItem(i)->getStateTopic());
        mqtt.subscribe(page->getItem(i)->getStateTopic());
      }
    }
  }
}

void onMqttDisconnected()
{
  mqttConnected = false;
  SMcontroler.drawHeader("Défaut serveur");
  DEBUGf("MQTT disconnected\n");
}

void onNumberCommand(HANumeric number, HANumber * sender)
{
  if (!number.isSet())
  {
    // the reset command was send by Home Assistant
  } else {
    lcd.setBrightness(number.toInt16());
  }
  sender->setState(number); // report the selected option back to the HA panel
}

JsonVariant findNestedKey(JsonObject obj, const char* key) {
  JsonVariant foundObject = obj[key];
  if (!foundObject.isNull())
    return foundObject;

  for (JsonPair pair : obj) {
    JsonVariant nestedObject = findNestedKey(pair.value(), key);
    if (!nestedObject.isNull())
      return nestedObject;
  }

  return JsonVariant();
}

void loadItems(Xpage* page, JsonArray itemList)
{
  int x = 0, y = 0, i = 0;
  int nb = itemList.size();
  const char* title;
  const char* targetMQTT;
  const char* targetPage;
  //  DEBUGf("      load %d items\n", nb);
  for (JsonVariant item : itemList)
  {
    if (!item.isNull())
    {
      x = (i % displayConfig.nbcols) * displayConfig.zonewidth;
      y = displayConfig.headerheight + (i / displayConfig.nbcols) * displayConfig.zoneheight;
      title = item["label"];
      targetMQTT = item["mqtt"];
      targetPage = item["page"];
      BtnAction bAction = (BtnAction)(int)item["action"].as<int>();
      bool itemAdded = false;
      if (bAction == BtnAction::HAtrigger)
      {
        //DEBUGf("        HAdevice %s\n", title);
        page->addItem(new XitemButton(i, x, y, displayConfig.zonewidth, displayConfig.zoneheight, COLOR_CYAN, title))
        ->setHAdevice();
        itemAdded = true;
      }
      if (bAction == BtnAction::ChangePage && strlen(targetPage) == 2)
      {
        //DEBUGf("        ChangePage %s\n", title);
        if (strlen(targetMQTT) > 0) {
          char* tmpMQTT = (char*)malloc(strlen(targetMQTT) + 7);
          strcpy(tmpMQTT, targetMQTT);
          if (item["page"].as<String>() == "kp")
          {
            //DEBUGf("          keypage ready\n");
            keypadPageNeeded = (char*)malloc(strlen(targetMQTT) + 5);
            strcpy(keypadPageNeeded, targetMQTT);
          }
          //DEBUGf("          add command\n");
          page->addItem(new XitemCommand(i, x, y, displayConfig.zonewidth, displayConfig.zoneheight, COLOR_CYAN, title, BtnAction::ChangePage))
          ->setMQTTconfig(tmpMQTT, item["source"].as<const char*>(), item["icon"].as<const char*>(), item["unit"].as<const char*>())
          ->setTargetPage(targetPage); // en dernier pour indiquer l'action "changePage"
          free(tmpMQTT);
          itemAdded = true;
        } else {
          //DEBUGf("          add button\n");
          page->addItem(new XitemButton(i, x, y, displayConfig.zonewidth, displayConfig.zoneheight, COLOR_CYAN, title))
          ->setTargetPage(targetPage);
          itemAdded = true;
        }
      }
      if (bAction == BtnAction::Display && strlen(targetMQTT) > 0)
      {
        //DEBUGf("        Display %s\n", title);
        page->addItem(new XitemInfo(i, x, y, displayConfig.zonewidth, displayConfig.zoneheight, COLOR_CYAN, title))
        ->setMQTTconfig(targetMQTT, item["source"].as<const char*>(), item["icon"].as<const char*>(), item["unit"].as<const char*>());
        itemAdded = true;
      }
      if (bAction == BtnAction::Command && strlen(targetMQTT) > 0)
      {
        //DEBUGf("        Command %s\n", title);
        page->addItem(new XitemCommand(i, x, y, displayConfig.zonewidth, displayConfig.zoneheight, COLOR_CYAN, title, BtnAction::Command))
        ->setMQTTconfig(targetMQTT, item["source"].as<const char*>(),  item["icon"].as<const char*>(), item["unit"].as<const char*>());
        itemAdded = true;
      }
      if (!itemAdded)
      {
        page->addItem(new Xitem(i, x, y, displayConfig.zonewidth, displayConfig.zoneheight, COLOR_CYAN, title));
      }
    }
    i++;
  }
}

void createKeypadPage(const char* target)
{
  uint8_t idx = 0;
  //DEBUGf("      create XpageKpad\n");
  Xpage* Xp = new XpageKeypad("kp", "Alarme", PageType::keypadPage, target);
  char pattern[] = "123456789*0#";
  char txt[] = "?";
  int padding_x = (displayConfig.width - displayConfig.kpadsize * 6) / 2;
  int padding_y = displayConfig.headerheight;
  for (byte r = 0; r < 4; r++) {
    for (byte c = 0; c < 3; c++) {
      txt[0] = pattern[r * 3 + c];
      int x = padding_x + (displayConfig.kpadsize * c);
      int y = padding_y + (displayConfig.kpadsize * r);
      Xp->addItem(new XitemKPad(idx++, x + 1, y + 1, displayConfig.kpadsize - 2, displayConfig.kpadsize - 2, COLOR_GRAY, txt, ACParmType::none));
    }
  }
  Xp->addItem(new XitemKPad(idx++, padding_x + 5 * displayConfig.kpadsize + 1, padding_y + (displayConfig.kpadsize * 0) + 1, displayConfig.kpadsize - 2, displayConfig.kpadsize - 2, COLOR_GRAY, "/i/d60-acp-armed_away.png", ACParmType::armAway));
  Xp->addItem(new XitemKPad(idx++, padding_x + 5 * displayConfig.kpadsize + 1, padding_y + (displayConfig.kpadsize * 1) + 1, displayConfig.kpadsize - 2, displayConfig.kpadsize - 2, COLOR_GRAY, "/i/d60-acp-armed_night.png", ACParmType::armNight));
  Xp->addItem(new XitemKPad(idx++, padding_x + 5 * displayConfig.kpadsize + 1, padding_y + (displayConfig.kpadsize * 3) + 1, displayConfig.kpadsize - 2, displayConfig.kpadsize - 2, COLOR_GRAY, "/i/d60-acp-disarmed.png", ACParmType::disarm));
  SMcontroler.addPage(Xp);
}

void LoadConfig()
{
  DEBUGln("Loading config file");
  File file = SPIFFS.open("/config.json");
  if (!file || file.isDirectory())
  {
    DEBUGln("failed to open config file");
    return;
  }
  DeserializationError error = deserializeJson(docJson, file);
  file.close();
  if (error) {
    DEBUGln("failed to deserialize config file");
    return;
  }
  displayConfig.nbcols = docJson["col"];
  displayConfig.nbrows = docJson["row"];
  displayConfig.zonewidth = displayConfig.width / displayConfig.nbcols;
  displayConfig.zoneheight = (displayConfig.height - displayConfig.headerheight) / displayConfig.nbrows;
  //DEBUGf("C:%d R:%d, Zw:%d, Zh:%d\n", displayConfig.nbcols, displayConfig.nbrows, displayConfig.zonewidth, displayConfig.zoneheight);
  JsonVariant pages = docJson["pages"];
  if (pages.is<JsonArray>())
  {
    int nbItems = pages.as<JsonArray>().size();
    //DEBUGf("  pages array\n");
    for (JsonVariant page : pages.as<JsonArray>())
    {
      PageType ptype = (PageType)(int)page["type"];
      //DEBUGf("    type:%d\n", (int)ptype);
      Xpage* Xp;
      //DEBUGf("      create Xpage\n");
      Xp = new Xpage(page["id"], page["title"], ptype);
      if (page.containsKey("items") && page["items"].is<JsonArray>())
      {
        loadItems(Xp, page["items"].as<JsonArray>());
      }
      SMcontroler.addPage(Xp);
    }
    if (keypadPageNeeded)
    {
      createKeypadPage(keypadPageNeeded);
      free(keypadPageNeeded);
    }
  }
  SMcontroler.addPage(new XpageDialog("dp", "Dialog", PageType::dialogPage));
  //
  JsonVariant hardware = docJson["gpio"];
  if (hardware.is<JsonArray>())
  {
    //DEBUGf("  hardware array\n");
    for (JsonVariant elt : hardware.as<JsonArray>())
    {
      if (nbGPIO < MAX_GPIO && elt["pin"].as<int>() > 0 && strlen(elt["name"].as<const char*>()) > 0)
      {
        HardwareType ht = (HardwareType)elt["type"].as<int>();
        GPIOs[nbGPIO].create(AP_ssid, elt["pin"].as<int>(), ht, elt["name"].as<const char*>());
        if (ht == HardwareType::numberSensor && strlen(elt["coefa"].as<const char*>()) > 0)
        {
          GPIOs[nbGPIO].setCoef(elt["coefa"].as<float>(), elt["coefb"].as<float>());
        }
        nbGPIO++;
      }
    }
  }
  isConfigLoaded = true;
  docJson.clear();
  DEBUGln("Config ready");
}
