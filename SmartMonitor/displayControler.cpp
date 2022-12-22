/*

*/
#include "display.h"
#include "SPIFFS.h"
#include "FS.h"
#include <WiFi.h>

static uint8_t tmpIcon[MAX_ICON_SIZE];

LGFX lcd;
LGFX_Sprite header(&lcd);
LGFX_Sprite zone(&lcd);
LGFX_Sprite kpad(&lcd);

Xcontroler SMcontroler;

displayConfig_t displayConfig;
uint8_t displayOrientation = 1;
bool mqttConnected = false;

Xcontroler::Xcontroler()
{
  _oldTimeDefaultPage = 0;
  _defaultPage = NULL;
  _dlgPage = NULL;
  _nbPages = 0;
  _msgWp = 0;
  _msgRp = 0;
  _msgCount = 0;
}

void Xcontroler::init()
{
  lcd.init();
  lcd.setRotation(displayOrientation);
  displayConfig.width = lcd.width();
  displayConfig.height = lcd.height();
  //DEBUGf("display O:%d W:%d H:%d\n", lcd.getRotation(), displayConfig.width, displayConfig.height);
  lcd.setColorDepth(24);
  if (lcd.width() >= 480)
  {
    lcd.setFont(&font_XS);
  } else {
    lcd.setFont(&font_XXS);
  }
  lcd.setCursor(0, 0);

  size_t nb = readFile("/i/flashscreen.png");
  if (nb)
  {
    lcd.drawPng(tmpIcon, nb, lcd.width() / 2 - 64, 0);
    lcd.display();
    lcd.setCursor(0, 128);
  }

  displayConfig.headerheight = 20;
  displayConfig.cornerradius = CORNER_RADIUS;
  displayConfig.kpadsize = min( displayConfig.width / 6, (displayConfig.height - displayConfig.headerheight) / 4);
}

void Xcontroler::drawBackground()
{
  auto transpalette = 0;
  //header.setColorDepth(lgfx::palette_4bit);
  header.createSprite(displayConfig.width, displayConfig.headerheight);
  header.fillScreen(transpalette);
  if (header.fontHeight(&font_XS) < displayConfig.headerheight)
  {
    header.setFont(&font_XS);
  } else
  {
    header.setFont(&font_XXS);
  }

  //zone.setColorDepth(lgfx::palette_4bit);
  DEBUGf("zone W:%d H:%d\n", displayConfig.zonewidth, displayConfig.zoneheight);
  zone.createSprite(displayConfig.zonewidth, displayConfig.zoneheight);
  zone.fillScreen(transpalette);
  zone.setFont(&font_L);
  zone.setTextColor(COLOR_ORANGE);
  zone.setTextDatum(lgfx::middle_center);

  //kpad.setColorDepth(lgfx::palette_4bit);
  kpad.createSprite(displayConfig.kpadsize, displayConfig.kpadsize);
  kpad.fillScreen(transpalette);
  kpad.setFont(&font_M);
  kpad.setTextDatum(lgfx::middle_center);

  lcd.clear(TFT_BLACK);
  header.drawLine(0 , displayConfig.headerheight - 1, displayConfig.width, displayConfig.headerheight - 1, COLOR_GRAY);
  header.setTextDatum(lgfx::top_left);
  header.setTextColor(COLOR_GRAY);
  header.pushSprite(&lcd, 0, 0);
  for (byte p = 0; p < _nbPages; p++)
  {
    _listPages[p]->draw();
  }
  lcd.display();
}

void Xcontroler::drawHeader(char* dateheure)
{
  //String level = "▏▁▂▃▅▆▇█ ";
  int wifiQuality = max( min(100, 2 * (WiFi.RSSI() + 100)), 0);
  //DEBUGf("wifi : %d %s\n", wifiQuality, dateheure);
  header.clear();
  header.drawLine(0 , displayConfig.headerheight - 1, displayConfig.width, displayConfig.headerheight - 1, COLOR_GRAY);
  header.setTextColor(mqttConnected ? COLOR_GRAY : COLOR_RED);
  header.setTextDatum(lgfx::top_left);
  int xDate = 0;
#ifndef SMALL_SCREEN
  header.drawString("SmartMonitor", 0, 0);
  header.setTextDatum(lgfx::top_center);
  xDate = displayConfig.width / 2;
#endif
  header.drawString(dateheure, xDate, 0);
  header.setTextDatum(lgfx::top_right);
  header.drawString(String(wifiQuality) + "%", displayConfig.width, 0);
  header.pushSprite(&lcd, 0, 0);
}

Xitem* Xcontroler::detectTouch()
{
  Xitem* item = NULL;
  static Xitem* firstPressedItem = NULL;
  static Xitem* lastPressedItem = NULL;
  static bool headerTapActived = false;

  static int32_t x, y;
  if (lcd.getTouch(&x, &y))
  {
    for (byte p = 0; p < _nbPages && item == NULL; p++)
    {
      for (byte i = 0; i < _listPages[p]->nbItems() && item == NULL; i++)
      {
        if (_listPages[p]->getItem(i)->isTouch(x, y))
        {
          item = _listPages[p]->getItem(i);
        }
      }
    }
    if (firstPressedItem == NULL && item) {
      item->draw(true);
      firstPressedItem = lastPressedItem = item;
    }
    if (item != lastPressedItem) {
      if (lastPressedItem) {
        lastPressedItem->draw(false);
      }
      if (item && (item == firstPressedItem)) {
        item->draw(true);
      }
      lastPressedItem = item;
    }
    return NULL; // tant qu'on est appuyé on renvoi NULL
  } else {
    if (lastPressedItem) {
      lastPressedItem->draw(false);
    }
    item = (firstPressedItem == lastPressedItem) ? lastPressedItem : NULL;
    lastPressedItem = firstPressedItem = NULL;
    return item;
  }
  return NULL;
}

void Xcontroler::addPage(Xpage* page)
{
  _listPages = (Xpage**)realloc(_listPages, (_nbPages + 1) * sizeof(Xpage*));
  if (_listPages == nullptr)
    return;

  _listPages[_nbPages] = page;
  _nbPages++;
  if (_defaultPage == NULL) {
    _defaultPage = page;
    page->show();
  }
  if (page->getPageType() == PageType::dialogPage)
  {
    _dlgPage = (XpageDialog*)page;
  }
}

int Xcontroler::getNbPages()
{
  return _nbPages;
}

Xpage* Xcontroler::getPage(int index)
{
  if (index < _nbPages)
  {
    return _listPages[index];
  }
  return NULL;
}

bool Xcontroler::resetTempoPage(Xpage* page)
{
  // TODO : vérif
  if (page != _listPages[0] && page != _defaultPage) {
    _oldTimeDefaultPage = millis();
    return true;
  }
  return false;
}

bool Xcontroler::doChangePage(const char* pageID)
{
  Xpage* targetPage = NULL;
  for (int p = 0; p < _nbPages; p++)
  {
    _listPages[p]->hide();
    if (strcmp(pageID, _listPages[p]->getID()) == 0)
    {
      targetPage = _listPages[p];
    }
  }
  if (targetPage != NULL)
  {
    if (targetPage->getPageType() == PageType::keypadPage)
    {
      ((XpageKeypad*)targetPage)->clearCode();
    }
    targetPage->show();
    targetPage->draw();
    lcd.display();
    _oldTimeDefaultPage = millis();
  } else {
    _defaultPage->show();
  }
}

void Xcontroler::doQuitPage(Xpage* page)
{
  page->hide();
  _oldTimeDefaultPage = 0;
  _defaultPage->show();
  _defaultPage->draw();
  lcd.display();
}

uint8_t* Xcontroler::getIconData()
{
  return tmpIcon;
}

void Xcontroler::loop()
{
  uint32_t t = millis();
  if (_defaultPage != NULL)
  {
    if (_oldTimeDefaultPage > 0 && (t > _oldTimeDefaultPage + TEMPO_DEFAULT_PAGE || t < _oldTimeDefaultPage))
    {
      for (int p = 1; p < _nbPages; p++)
      {
        _listPages[p]->hide();
        _listPages[p]->draw();
      }
      _oldTimeDefaultPage = 0;
      _defaultPage->show();
      _defaultPage->draw();
      lcd.display();
    }
  }
  Xitem* item = detectTouch();
  if (item != NULL) {
    DEBUGf("touch:%s\n", item->getTitle());
    item->doTouch();
  }
  mqttMsgPop();
}

void Xcontroler::setScratchMode()
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.setTextColor(COLOR_WHITE);
  lcd.setFont(&font_S);
  lcd.println("Touch again to restart\n");
  lcd.setFont(&font_XXS);
  lcd.println("Taille XXS");
  lcd.setFont(&font_XS);
  lcd.println("Taille XS");
  lcd.setFont(&font_S);
  lcd.println("Taille S");
  lcd.setFont(&font_M);
  lcd.println("Taille M");
  lcd.setFont(&font_L);
  lcd.println("Taille L");
  lcd.setFont(&font_XL);
  lcd.println("Taille XL");
  lcd.setFont(&font_XXL);
  lcd.println("Taille XXL");
}

void Xcontroler::dialog(JsonArray texts)
{
  if (_dlgPage != NULL)
  {
    _dlgPage->show();
    _dlgPage->drawText(texts);
    lcd.display();
    _oldTimeDefaultPage = millis();
  }
}

size_t Xcontroler::readFile(const char * path)
{
  DEBUGf("Reading file: %s\n", path);
  size_t result = 0;
  File file = SPIFFS.open(path);
  if (!file || file.isDirectory())
  {
    DEBUGf("failed to open icon : %s\n", path);
  }
  size_t numBytesToRead = file.size();
  //DEBUGf("size:%d\n", numBytesToRead);
  if (numBytesToRead >= MAX_ICON_SIZE)
  {
    DEBUGf("icon too big : %s\n", path);
  }
  if (file.readBytes((char*)tmpIcon, numBytesToRead))
  {
    result = numBytesToRead;
  }
  file.close();
  return result;
}

bool Xcontroler::screenDump(void)
{
  std::size_t dlen;
  std::uint8_t* png = (std::uint8_t*)lcd.createPng(&dlen, 0, 0, 10, 10);
  if (!png)
  {
    DEBUGln("error:createPng");
    return false;
  }
  bool result = false;
  char imagefile[30];
  sprintf(imagefile, "/dump/screendump%d.png", 0);
  File file = SPIFFS.open(imagefile, "w");
  if (file)
  {
    file.write((std::uint8_t*)png, dlen);
    file.close();
    DEBUGf("Dump %d %s ok\n", dlen, imagefile);
    result = true;
  }
  else
  {
    DEBUGln("error:file open failure");
  }
  free(png);
  return result;
}

void Xcontroler::mqttMsgPush(const char* topic, const char* payload)
{
  if (_msgCount < FIFO_MSG_SIZE)
  {
    mqttMsg msg;
    msg.create(topic, payload);
    _msgBuf[_msgWp] = msg;
    _msgCount++;
    _msgWp = (_msgWp + 1) % FIFO_MSG_SIZE;
  }
}

void Xcontroler::mqttMsgPop()
{
  if (_msgCount > 0)
  {
    mqttProcess(&_msgBuf[_msgRp]);
    //
    _msgBuf[_msgRp].erase();
    _msgCount--;
    _msgRp = (_msgRp + 1) % FIFO_MSG_SIZE;
  }
}

void Xcontroler::mqttProcess(mqttMsg* msg)
{
  //DEBUGf("Process(%d) -> %s = %s\n", _msgCount, msg->topic, msg->payload);
  if (memcmp(msg->topic, "smartmonitor/alarm_message", strlen(msg->topic)) == 0)
  {
    //DEBUGf("MSG:%s\n", pl);
    if (strlen(msg->payload)) {
      DynamicJsonDocument docJSon(JSON_MAX_SIZE);
      deserializeJson(docJSon, msg->payload);
      if (docJSon.containsKey("text") && docJSon["text"].is<JsonArray>())
      {
        SMcontroler.dialog(docJSon["text"].as<JsonArray>());
      }
      docJSon.clear();
    }
    return;
  } else if (memcmp(msg->topic, "smartmonitor/dateheure", strlen(msg->topic)) == 0)
  {
    SMcontroler.drawHeader(msg->payload);
    return;
  } else
  {
    for (int p = 0; p < SMcontroler.getNbPages(); p++)
    {
      Xpage* page = SMcontroler.getPage(p);
      for (int i = 0; i < page->nbItems(); i++)
      {
        XitemInfo* item = (XitemInfo*)page->getItem(i);
        if (item->isSuscribeMQTT())
        {
          if (memcmp(msg->topic, item->getStateTopic(), strlen(msg->topic)) == 0)
          {
            if (msg->payload[0] == '{' || msg->payload[0] == '[')
            {
              if (item->isStateAttribute())
              {
                //DEBUGf("    Json found\n");
                char* stateAttrib = item->getStateAttribute();
                DynamicJsonDocument docJSon(JSON_PAYLOAD_SIZE);
                deserializeJson(docJSon, (const char*)msg->payload); // const char* obligatoire pour read only
                if (docJSon.is<JsonArray>())
                {
                  //DEBUGf("      JSon array\n");
                  int num = 0;
                  String datalist = "";
                  for (JsonVariant plItem : docJSon.as<JsonArray>())
                  {
                    if (plItem.is<JsonObject>())
                    {
                      if (num > 0) {
                        datalist += "|";
                      }
                      for (JsonPair kv : plItem.as<JsonObject>())
                      {
                        if (strcmp(kv.key().c_str(), stateAttrib) == 0)
                        {
                          if (kv.value().is<int>())
                          {
                            //DEBUGf("      set %d int data to %s : %d\n", num, stateAttrib, kv.value().as<int>());
                            datalist += String(kv.value().as<int>());
                          } else if (kv.value().is<float>())
                          {
                            //DEBUGf("      set %d float data to %s : %f\n", num, stateAttrib, kv.value().as<float>());
                            datalist += String(kv.value().as<float>(), 1);
                          } else if (kv.value().is<char*>())
                          {
                            //DEBUGf("      set %d char* data to %s : %s\n", num, stateAttrib, kv.value().as<char*>());
                            datalist += String(kv.value().as<char*>());
                          }
                          num++;
                        }
                      }
                    } else {
                      // TODO tableau de valeurs, pas encore vu
                    }
                  }
                  //
                  item->setData(datalist, num);
                } else if (docJSon.is<JsonObject>())
                {
                  //DEBUGf("      JSon object\n");
                  JsonObject root = docJSon.as<JsonObject>();
                  for (JsonPair kv : root) {
                    if (strcmp(kv.key().c_str(), stateAttrib) == 0)
                    {
                      if (kv.value().is<int>())
                      {
                        //DEBUGf("      set int data to %s : %d\n", stateAttrib, kv.value().as<int>());
                        item->setData(String(kv.value().as<int>()));
                      } else if (kv.value().is<float>())
                      {
                        //DEBUGf("      set float data to %s : %f\n", stateAttrib, kv.value().as<float>());
                        item->setData(String(kv.value().as<float>()));
                      } else if (kv.value().is<char*>())
                      {
                        //DEBUGf("      set char* data to %s : %s\n", stateAttrib, kv.value().as<char*>());
                        item->setData(String(kv.value().as<char*>()));
                      }
                    }
                  }
                }
                docJSon.clear();
              } else {
                item->setData("no attrib");
              }
            }
            else
            {
              item->setData(String(msg->payload));
            }
          }
        }
      }
    }
  }
}

String getValue(String data, char separator, int index)
{
  int found = 0;
  int strIndex[] = { 0, -1 };
  int maxIndex = data.length() - 1;

  for (int i = 0; i <= maxIndex && found <= index; i++) {
    if (data.charAt(i) == separator || i == maxIndex) {
      found++;
      strIndex[0] = strIndex[1] + 1;
      strIndex[1] = (i == maxIndex) ? i + 1 : i;
    }
  }
  return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}

size_t strends(const char* str1, const char* str2)
{
  size_t res = 0;
  if (str1 != NULL && str2 != NULL) {
    while (str1[res] != 0 && str2[res] != 0 && str1[res] == str2[res]) {
      res++;
    }
  }
  return res;
}
