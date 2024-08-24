/*
  Designed for ESP32 only
*/
#include <ESPAsyncWebServer.h>
#include <ESPmDNS.h>
#include <EEPROM.h>
#include <Update.h>

#ifdef USE_ETHERNET
#include <ETH.h>
#endif

#include "display.h"

#include "include_html.hpp"

extern LGFX lcd;

extern Xcontroler SMcontroler;
extern displayConfig_t displayConfig;
extern uint8_t displayOrientation;
extern bool needConfigLoading;

char Wifi_ssid[EEPROM_TEXT_SIZE] = "";  // WiFi SSID
char Wifi_pass[EEPROM_TEXT_SIZE] = "";  // WiFi password

char AP_ssid[10] = "smartmon0";  // AP WiFi SSID
char AP_pass[9] = "12345678";   // AP WiFi password

char mqtt_host[EEPROM_TEXT_SIZE] = "192.168.0.100";
char mqtt_user[EEPROM_TEXT_SIZE] = "mqttuser";
char mqtt_pass[EEPROM_TEXT_SIZE] = "mqttpass";

char LumDevID[] = "DisplayBrightness0";  // Device unique ID

static bool eth_connected = false;

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

bool wifiok = false;

StaticJsonDocument<JSON_MAX_SIZE> docJson;

String fillMQTT(XitemInfo* item, String &res)
{
  res = "";
  if (item->isSuscribeMQTT())
  {
    res += String(item->getBaseTopic());
    size_t p = strends(item->getStateTopic(), item->getBaseTopic());
    if (p <= strlen(item->getStateTopic()))
    {
      res += ";" + String(item->getStateTopic()).substring(p);
      if (item->isStateAttribute())
      {
        res += "|" + String(item->getStateAttribute());
      }
    } else
    {
      res += ";";
    }
    p = strends(item->getCommandTopic(), item->getBaseTopic());
    if (item->getCommandTopic() != NULL && p < strlen(item->getCommandTopic()))
    {
      res += ";" + String(item->getCommandTopic()).substring(p);
    }
  }
  if (res.endsWith(";")) {
    res.remove(res.length() - 1);
  }
  return res;
}

String getHTMLforPage(int numpage, Xpage* page)
{
  String blocP = html_page;
  blocP.replace("#P#", String(numpage));
  blocP.replace("%CNFID%", page != NULL ? page->getID() : "p" + String(numpage));
  blocP.replace("%CNFTITLE%", page != NULL ? page->getTitle() : "");

  blocP.replace("%CNFP_T1%", "disabled"); // allways disable keypadPage
  if (numpage < 1) {
    blocP.replace("%CNFP_T0%", "selected disabled");
  } else {
    blocP.replace("%CNFP_T0%",  page == NULL || (int)page->getPageType() == 0 ? "selected" : "");
  }
  //
  String replaceGen = "";
  int maxItem = displayConfig.nbcols * displayConfig.nbrows;
  for (int i = 0; i < maxItem; i++)
  {
    //replaceGen += "<div id='confitems_" + String(numpage) + "_" + String(i) + "' class='flex-wrap'>chargement...</div>\n";
    replaceGen += "<tr id='confitems_" + String(numpage) + "_" + String(i) + "'></tr>\n";
  }
  //
  blocP.replace("%GENITEMS%", replaceGen);
  return blocP;
}

String getHTMLforItems(int numpage, Xpage* page, int numitem)
{
  String blocI = html_item;
  blocI.replace("#P#", String(numpage));
  blocI.replace("#I#", String(numitem));
  bool lineok = false;
  if (page != NULL)
  {
    XitemInfo* item = (XitemInfo*)page->getItemByIndex(numitem);
    if (item != NULL)
    {
      blocI.replace("%CNFLABEL%", item->getTitle());
      String res;
      for (int n = 0; n < (int)BtnAction::lastBtnAction; n++)
      {
        String kcnf = "%CNFI_A" + String(n) + "%";
        blocI.replace(kcnf, (int)item->getAction() == n ? "selected" : "");
      }
      switch (item->getAction())
      {
        case BtnAction::HAtrigger:
          blocI.replace("%CNFMQTT%", "");
          blocI.replace("%CNFPAGE%", "");
          break;
        case BtnAction::ChangePage:
          blocI.replace("%CNFMQTT%", fillMQTT(item, res)); // TODO
          blocI.replace("%CNFPAGE%", item->getTargetPage());
          break;
        case BtnAction::Display:
          blocI.replace("%CNFMQTT%", fillMQTT(item, res)); // TODO
          blocI.replace("%CNFPAGE%", "");
          break;
        case BtnAction::Command:
          blocI.replace("%CNFMQTT%", fillMQTT(item, res)); // TODO
          blocI.replace("%CNFPAGE%", "");
          break;
        default:
          blocI.replace("%CNFMQTT%", "");
          blocI.replace("%CNFPAGE%", "");
          break;
      }
      blocI.replace("%CNFSRC%", String(item->getSource()));
      blocI.replace("%CNFICON%", String(item->getIcon()));
      blocI.replace("%CNFUNIT%", String(item->getUnit()));
      //
      blocI.replace("%CNFI_MQTTOFF%", ((int)item->getAction() == 2 || (int)item->getAction() == 3 || (int)item->getAction() == 4) ? "" : "disabled");
      blocI.replace("%CNFI_SRCOFF%", item->isSuscribeMQTT() ? "" : "disabled");
      blocI.replace("%CNFI_ICONOFF%", item->isSuscribeMQTT() ? "" : "disabled");
      blocI.replace("%CNFI_UNITOFF%", item->isSuscribeMQTT() ? "" : "disabled");
      blocI.replace("%CNFI_PAGEOFF%", ((int)item->getAction() == 2) ? "" : "disabled");
      //blocI.replace("%CNFTYPE%", String((int)item->getDataType()));
      lineok = true;
    }
  }
  if (!lineok)
  {
    blocI.replace("%CNFLABEL%", "");
    for (int n = 0; n < (int)BtnAction::lastBtnAction; n++)
    {
      String kcnf = "%CNFI_A" + String(n) + "%";
      blocI.replace(kcnf, (int)BtnAction::None == n ? "selected" : "");
    }
    blocI.replace("%CNFMQTT%", "");
    blocI.replace("%CNFSRC%", "");
    blocI.replace("%CNFICON%", "");
    blocI.replace("%CNFUNIT%", "");
    blocI.replace("%CNFPAGE%", "");
    blocI.replace("%CNFI_MQTTOFF%", "disabled");
    blocI.replace("%CNFI_SRCOFF%", "disabled");
    blocI.replace("%CNFI_ICONOFF%", "disabled");
    blocI.replace("%CNFI_UNITOFF%", "disabled");
    blocI.replace("%CNFI_PAGEOFF%", "disabled");
  }
  return blocI;
}

void sendConfigPage()
{
  String blocPage = "";
  for (int p = 0; p < SMcontroler.getNbPages(); p++)
  {
    Xpage* page = SMcontroler.getPage(p);
    if (page->getPageType() == PageType::normalPage)
    {
      blocPage += getHTMLforPage(p, page);
    }
  }
  docJson.clear();
  docJson["cmd"] = "html";
  docJson["#conf_page"] = blocPage;
  String js;
  serializeJson(docJson, js);
  ws.textAll(js);
}

void sendConfigItems()
{
  for (int p = 0; p < SMcontroler.getNbPages(); p++)
  {
    int maxItem = displayConfig.nbcols * displayConfig.nbrows;
    Xpage* page = SMcontroler.getPage(p);
    if (page->getPageType() == PageType::normalPage) {
      int i;
      for (i = 0; i < maxItem; i++)
      {
        docJson.clear();
        docJson["cmd"] = "html";
        docJson["#confitems_" + String(p) + "_" + String(i) ] = getHTMLforItems(p, page, i);
        String js;
        serializeJson(docJson, js);
        ws.textAll(js);
      }
    }
  }
}

void notifyConfig() {
  if (ws.availableForWriteAll()) {
    sendConfigPage();
    sendConfigItems();
    ws.textAll("{\"cmd\":\"loadselect\"}");
  }
}

boolean isValidNumber(String str) {
  for (byte i = 0; i < str.length(); i++)
  {
    if (!isDigit(str.charAt(i)))
    {
      return false;
    }
  }
  return str.length() > 0;
}

void onIndexRequest(AsyncWebServerRequest *request) {

  AsyncResponseStream *response = request->beginResponseStream("text/html");
  File f = SPIFFS.open("/index.html", "r");
  if (f) {
    String html;
    while (f.available()) {
      html = f.readStringUntil('\n') + '\n';
      if (html.indexOf('%') > 0)
      {
        if (html.indexOf("%GENHTMLPAGE%") > 0)
        {
          //
          String blocI = html_header;
          blocI.replace("%CNFCOL%", String(displayConfig.nbcols));
          blocI.replace("%CNFROW%", String(displayConfig.nbrows));
          html.replace("<!--%GENHTMLPAGE%-->", blocI);
        } else if (html.indexOf("%GENHTMLGPIO%") > 0)
        {
          String blocGPIO = "";
          for (int n = 0; n < MAX_GPIO; n++)
          {
            String blocH = html_hardware;
            blocH.replace("#H#", String(n));
            // pin list
            String blocPins = "";
            for (byte p = 0; p < sizeof(pinAvailable); p++)
            {
              String sPin = String(pinAvailable[p]);
              blocPins += "<option value='" + sPin + "' " + (GPIOs[n]._pin == pinAvailable[p] ? "selected" : "") + ">" + sPin + "</option>";
            }
            blocH.replace("%CNFH_Pnone%", (GPIOs[n]._pin == -1 ? "selected" : ""));
            blocH.replace("%CNFH_PINLIST%", blocPins);
            blocH.replace("%CNFNAME%", String(GPIOs[n]._name));
            for (int t = 0; t < (int)HardwareType::lastHardwareType; t++) {
              String tcnf = "%CNFH_T" + String(t) + "%";
              blocH.replace(tcnf, (int)GPIOs[n]._htype == t ? "selected" : "");
            }
            blocH.replace("%CNFH_COEFOFF%", GPIOs[n]._htype == HardwareType::numberSensor ? "" : "disabled");
            blocGPIO += blocH;
          }
          html.replace("<!--%GENHTMLGPIO%-->", blocGPIO);
        } else
        {
          html.replace("%CNFCOL%", String(displayConfig.nbcols));
          html.replace("%CNFROW%", String(displayConfig.nbrows));
          html.replace("%CNFOR0%", displayOrientation == 0 ? "checked" : "");
          html.replace("%CNFOR1%", displayOrientation == 1 ? "checked" : "");
          html.replace("%CNFOR2%", displayOrientation == 2 ? "checked" : "");
          html.replace("%CNFOR3%", displayOrientation == 3 ? "checked" : "");
          html.replace("%CNFCODE%", String(AP_ssid[8]));
          html.replace("%CNFMAC%", WiFi.macAddress());
          html.replace("%CNFSSID%", Wifi_ssid);
          html.replace("%CNFPWD%", Wifi_pass);
          html.replace("%CNFMQTTSRV%", mqtt_host);
          html.replace("%CNFMQTTUSER%", mqtt_user);
          html.replace("%CNFMQTTPASS%", mqtt_pass);
          html.replace("%VERSION%", SM_VERSION);
          //
        }
      }
      response->print(html);
    }
    f.close();
  }
  request->send(response);
}

void onConfigRequest(AsyncWebServerRequest * request)
{
  if (request->hasParam("cnf", true))
  {
    String cnf = request->getParam("cnf", true)->value();
    //    DEBUGln(cnf);
    if (cnf == "conf")
    {
      DynamicJsonDocument docJSon(JSON_MAX_SIZE);
      JsonObject Jconfig = docJSon.to<JsonObject>();
      int params = request->params();
      for (int i = 0; i < params; i++)
      {
        AsyncWebParameter* p = request->getParam(i);
        if (p->isPost() && p->name() != "cnf")
        {
          //DEBUGf("_POST[%s]: %s\n", p->name().c_str(), p->value().c_str());
          String str = p->name();
          if (str == "col") {
            Jconfig["col"] = p->value();
          } else if (str == "row") {
            Jconfig["row"] = p->value();
          } else {
            // exemple : pages_action_0_2
            // items[2].action of pages[0]
            //      or : pages_title_0
            // title of pages[0]
            String keybloc = getValue(str, '_', 0);
            String key = getValue(str, '_', 1);
            String kline = getValue(str, '_', 2);
            String kitem = getValue(str, '_', 3);
            String sval(p->value().c_str());

            //DEBUGf("[%s][%s][%s](%s) = %s\n", keybloc, kline, key, kitem, sval);
            if (kitem == "")
            {
              if (isValidNumber(sval))
              {
                Jconfig[keybloc][kline.toInt()][key] = sval.toInt();
              } else
              {
                Jconfig[keybloc][kline.toInt()][key] = sval;
              }
            } else
            {
              // on laisse le Titre vide pour supprimer une ligne
              if (request->getParam("pages_label_" + kline + "_" + kitem, true)->value() != "")
              {
                if (isValidNumber(sval))
                {
                  Jconfig[keybloc][kline.toInt()]["items"][kitem.toInt()][key] = sval.toInt();
                } else
                {
                  Jconfig[keybloc][kline.toInt()]["items"][kitem.toInt()][key] = sval;
                }
              }
            }
          }
        }
      }
      // nettoyage des pages vides
      for (JsonArray::iterator it = Jconfig["pages"].as<JsonArray>().begin(); it != Jconfig["pages"].as<JsonArray>().end(); ++it)
      {
        if (!(*it).containsKey("items"))
        {
          Jconfig["pages"].as<JsonArray>().remove(it);
        }
      }
      //
      Jconfig["end"] = true;
      String Jres;
      size_t Lres = serializeJson(docJSon, Jres);
      //DEBUGln(Jres);

      File file = SPIFFS.open("/config.json", "w");
      if (file)
      {
        file.write((byte*)Jres.c_str(), Lres);
        file.close();
        DEBUGln("Fichier de config enregistré");
        //
        needConfigLoading = true;
      } else {
        DEBUGln("Erreur sauvegarde config");
      }
    }
    if (cnf == "wifi")
    {
      displayOrientation = request->getParam("ORIENTATION", true)->value().toInt() % 4;
      AP_ssid[8] = request->getParam("CODE", true)->value().c_str()[0];
      LumDevID[17] = AP_ssid[8];
      strcpy(Wifi_ssid, request->getParam("SSID", true)->value().c_str() );
      strcpy(Wifi_pass, request->getParam("PWD", true)->value().c_str() );
      strcpy(mqtt_host, request->getParam("MQTTSRV", true)->value().c_str() );
      strcpy(mqtt_user, request->getParam("MQTTUSER", true)->value().c_str() );
      strcpy(mqtt_pass, request->getParam("MQTTPASS", true)->value().c_str() );

      DEBUGln(displayOrientation);
      DEBUGln(AP_ssid);
      DEBUGln(Wifi_ssid);
      DEBUGln(Wifi_pass);
      DEBUGln(mqtt_host);
      DEBUGln(mqtt_user);
      DEBUGln(mqtt_pass);

      EEPROM.write(0, displayOrientation);
      EEPROM.writeChar(1, AP_ssid[8]);
      EEPROM.writeString(EEPROM_TEXT_OFFSET, Wifi_ssid);
      EEPROM.writeString(EEPROM_TEXT_OFFSET + (EEPROM_TEXT_SIZE * 1), Wifi_pass);
      EEPROM.writeString(EEPROM_TEXT_OFFSET + (EEPROM_TEXT_SIZE * 2), mqtt_host);
      EEPROM.writeString(EEPROM_TEXT_OFFSET + (EEPROM_TEXT_SIZE * 3), mqtt_user);
      EEPROM.writeString(EEPROM_TEXT_OFFSET + (EEPROM_TEXT_SIZE * 4), mqtt_pass);
      EEPROM.commit();
    }
  }
  request->send(200, "text/plain", "OK");
}

size_t content_len;
void handleDoUpdate(AsyncWebServerRequest *request, const String& filename, size_t index, uint8_t *data, size_t len, bool final)
{
  if (!index)
  {
    DEBUGln("Update start");
    content_len = request->contentLength();
    // if filename includes spiffs, update the spiffs partition
    int cmd = (filename.indexOf("spiffs") > -1) ? U_SPIFFS : U_FLASH;
    if (!Update.begin(UPDATE_SIZE_UNKNOWN, cmd))
    {
      Update.printError(Serial);
    }
  }
  if (Update.write(data, len) != len)
  {
    Update.printError(Serial);
  }

  if (final)
  {
    AsyncWebServerResponse *response = request->beginResponse(302, "text/plain", "Please wait while the device reboots");
    response->addHeader("Refresh", "20");
    response->addHeader("Location", "/");
    request->send(response);
    if (!Update.end(true))
    {
      Update.printError(Serial);
    } else
    {
      DEBUGln("Update complete");
      delay(100);
      yield();
      delay(100);

      ESP.restart();
    }
  }
}

void handleDoFile(AsyncWebServerRequest *request, const String& filename, size_t index, uint8_t *data, size_t len, bool final)
{
  if (!index)
  {
    if (filename.endsWith(".png"))
    {
      request->_tempFile = SPIFFS.open("/i/" + filename, "w");
    } else if (filename.endsWith(".css"))
    {
      request->_tempFile = SPIFFS.open("/css/" + filename, "w");
    } else if (filename.endsWith(".js"))
    {
      request->_tempFile = SPIFFS.open("/js/" + filename, "w");
    } else
    {
      request->_tempFile = SPIFFS.open("/" + filename, "w");
    }
  }
  if (len)
  {
    request->_tempFile.write(data, len);
  }
  if (final)
  {
    request->_tempFile.close();
    request->redirect("/");
  }
}

void updateProgress(size_t prg, size_t sz)
{
  DEBUGf("Progress: %d%%\n", (prg * 100) / content_len);
  lcd.setCursor(lcd.width() - 85, 0);
  lcd.printf("Upl:%02d%%", (prg * 100) / content_len);
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len)
{
  DEBUGln("handleWebSocketMessage");
  AwsFrameInfo *info = (AwsFrameInfo*)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT)
  {
    data[len] = 0;
    DEBUGln((char*)data);
    String cmd = getValue((char*)data, ':', 0);
    String val = getValue((char*)data, ':', 1);
    if (cmd == "addpage")
    {
      docJson.clear();
      docJson["cmd"] = "replace";
      docJson["#newpage_" + val] = getHTMLforPage(val.toInt(), NULL);
      String js;
      serializeJson(docJson, js);
      ws.textAll(js);

      int maxItem = displayConfig.nbcols * displayConfig.nbrows;
      for (int i = 0; i < maxItem; i++)
      {
        docJson.clear();
        docJson["cmd"] = "html";
        docJson["#confitems_" + val + "_" + String(i)] = getHTMLforItems(val.toInt(), NULL, i);
        String js;
        serializeJson(docJson, js);
        ws.textAll(js);
      }
    }
    ws.textAll("{\"cmd\":\"loadselect\"}");

    return;
  }
}

void onEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void *arg, uint8_t *data, size_t len)
{
  switch (type) {
    case WS_EVT_CONNECT:
      DEBUGf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
      notifyConfig();
      break;
    case WS_EVT_DISCONNECT:
      DEBUGf("WebSocket client #%u disconnected\n", client->id());
      break;
    case WS_EVT_DATA:
      handleWebSocketMessage(arg, data, len);
      break;
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
      break;
  }
}

void WiFiEvent(WiFiEvent_t event)
{
  switch (event) {
    case ARDUINO_EVENT_WIFI_READY:
      DEBUGln("WiFi ready");
      break;
    case ARDUINO_EVENT_WIFI_STA_START:
      DEBUGln("WiFi search");
      break;
    case ARDUINO_EVENT_WIFI_STA_CONNECTED:
      DEBUGln("WiFi STA connected");
      break;
    case ARDUINO_EVENT_WIFI_STA_GOT_IP:
      DEBUGln("WiFi connected");
      DEBUG("IP address: ");
      DEBUGln(WiFi.localIP());
      wifiok = true;
      break;
    case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
      DEBUGln("WiFi lost connection");
      wifiok = false;
      WiFi.disconnect();
      break;
      // For Ethernet
#ifdef USE_ETHERNET
    case ARDUINO_EVENT_ETH_START:
      DEBUGln("ETH Started");
      //set eth hostname here
      ETH.setHostname(AP_ssid);
      break;
    case ARDUINO_EVENT_ETH_CONNECTED:
      DEBUGln("ETH Connected");
      break;
    case ARDUINO_EVENT_ETH_GOT_IP:
      DEBUG("ETH MAC: ");
      DEBUG(ETH.macAddress());
      DEBUG(", IPv4: ");
      DEBUG(ETH.localIP());
      if (ETH.fullDuplex()) {
        DEBUG(", FULL_DUPLEX");
      }
      DEBUG(", ");
      DEBUG(ETH.linkSpeed());
      DEBUGln("Mbps");
      eth_connected = true;
      break;
    case ARDUINO_EVENT_ETH_DISCONNECTED:
      DEBUGln("ETH Disconnected");
      eth_connected = false;
      break;
    case ARDUINO_EVENT_ETH_STOP:
      DEBUGln("ETH Stopped");
      eth_connected = false;
      break;
    default:
      DEBUGf("[WiFi-event] %d\n", event);
      break;
#endif
  }
}

bool WiFi_STA_TryConnect(bool lcdtrace)
{
  if (strlen(Wifi_ssid))
  {
    if (lcdtrace)
    {
      lcd.print("wifi ");
      lcd.print(Wifi_ssid);
    }
    int tentativeWiFi = 0;
    WiFi.mode(WIFI_STA);
    WiFi.begin(Wifi_ssid, Wifi_pass);
    while (WiFi.status() != WL_CONNECTED && tentativeWiFi < 20)
    {
      if (lcdtrace) lcd.print(".");
      delay(500);
      DEBUG( "." );
      tentativeWiFi++;
    }
    if (lcdtrace) lcd.println("");
    DEBUGln("");
    return (WiFi.status() == WL_CONNECTED);
  }
  return false;
}

void configWifi()
{
  char txt[40];

  WiFi.onEvent(WiFiEvent);
  DEBUGln(WiFi.macAddress());
  lcd.print("MAC: ");
  lcd.println(WiFi.macAddress());

#ifdef USE_ETHERNET
  ETH.begin();
  wifiok = true;
#else
  // Mode normal
  bool wifires = WiFi_STA_TryConnect(true);
  if (wifiok)
  {
    lcd.println("");
    lcd.println("Wifi OK");
    lcd.println(WiFi.localIP().toString().c_str());
  } else {
    DEBUGln("No wifi STA, set AP mode.");
    // Mode AP
    WiFi.mode(WIFI_AP);
    WiFi.softAP(AP_ssid, AP_pass);
    // Default IP Address is 192.168.4.1
    // if you want to change uncomment below
    // softAPConfig (local_ip, gateway, subnet)
    DEBUGln();
    DEBUGf("AP WIFI :%s\n", AP_ssid );
    DEBUGf("AP IP Address: %s\n", WiFi.softAPIP().toString().c_str());

    lcd.print("Wifi AP ");
    lcd.println(AP_ssid);
    lcd.print(WiFi.softAPIP().toString().c_str());
  }
#endif
  if (MDNS.begin(AP_ssid))
  {
    MDNS.addService("http", "tcp", 80);
    lcd.printf("web : %s.local\n", AP_ssid);
  }
}

void configWeb()
{
  server.serveStatic("/js", SPIFFS, "/js");
  server.serveStatic("/fonts", SPIFFS, "/fonts");
  server.serveStatic("/css", SPIFFS, "/css");
  server.serveStatic("/i", SPIFFS, "/i");
  server.serveStatic("/dump", SPIFFS, "/dump");
  server.serveStatic("/config.json", SPIFFS, "/config.json");
  server.on("/", HTTP_GET, onIndexRequest);
  server.on("/doconfig", HTTP_POST, onConfigRequest);
  server.on("/doupdate", HTTP_POST, [](AsyncWebServerRequest * request) {},
  [](AsyncWebServerRequest * request, const String & filename, size_t index, uint8_t *data, size_t len, bool final) {
    handleDoUpdate(request, filename, index, data, len, final);
  });
  server.on("/dofile", HTTP_POST, [](AsyncWebServerRequest * request) {
    request->send(200);
  },
  [](AsyncWebServerRequest * request, const String & filename, size_t index, uint8_t *data, size_t len, bool final) {
    handleDoFile(request, filename, index, data, len, final);
  });
  server.on("/restart", HTTP_GET, [] (AsyncWebServerRequest * request)
  {
    AsyncWebServerResponse *response = request->beginResponse(302, "text/plain", "Please wait while the device reboots");
    response->addHeader("Refresh", "10");
    response->addHeader("Location", "/");
    request->send(response);
    delay(500);
    ESP.restart();
  });
  server.on("/screendump", HTTP_GET, [] (AsyncWebServerRequest * request)
  {
    SMcontroler.screenDump();
    AsyncWebServerResponse *response = request->beginResponse(302, "text/plain", "Please wait while the device reboots");
    response->addHeader("Refresh", "5");
    response->addHeader("Location", "/");
    request->send(response);
  });
  server.begin();
  ws.onEvent(onEvent);
  server.addHandler(&ws);
  Update.onProgress(updateProgress);
  DEBUGln ("HTTP server started");
  lcd.println("web server ready");
}
