/*

*/
#include "display.h"

extern HAMqtt mqtt;
extern displayConfig_t displayConfig;
extern LGFX lcd;
extern Xcontroler SMcontroler;

Xpage::Xpage(const char* id, const char* title, PageType ptype)
{
  _visible = false;
  _color = COLOR_BLACK;
  _items = nullptr;
  _nbItems = 0;
  _x = 0;
  _y = displayConfig.headerheight;
  _w = displayConfig.width;
  _h = displayConfig.height - _y;
  _type = ptype;
  _id = (char*)malloc(strlen(id) + 1);
  strcpy(_id, id);
  _title = (char*)malloc(strlen(title) + 1);
  strcpy(_title, title);
}

void Xpage::draw()
{
  if (_visible)
  {
    LGFX_Sprite sp(&lcd);
    sp.setColorDepth(lgfx::palette_2bit);
    sp.createSprite(_w, _h);
    sp.fillRect(0, 0, _w, _h, COLOR_BLACK);
    sp.pushSprite(_x, _y);
    sp.deleteSprite();
    DEBUGf("Page %s draw items %d %d %d %d\n", _id, _x, _y, _w, _h);
    for (byte i = 0; i < _nbItems; i++)
    {
      _items[i]->draw(false);
    }
  }
}

Xitem* Xpage::addItem(Xitem* item)
{
  _items = (Xitem**)realloc(_items, (_nbItems + 1) * sizeof(Xitem*));
  if (_items == nullptr)
    return nullptr;

  _items[_nbItems] = item;
  item->setPage(this);
  _nbItems++;
  return item;
}

int Xpage::nbItems()
{
  return _nbItems;
}

Xitem* Xpage::getItem(int index)
{
  return index < _nbItems ? _items[index] : NULL;
}

Xitem* Xpage::getItemByIndex(uint8_t index)
{
  for (byte i = 0; i < _nbItems; i++)
  {
    if (_items[i]->getIndex() == index)
    {
      return _items[i];
    }
  }
  return NULL;
}

bool Xpage::isVisible()
{
  return _visible;
}

PageType Xpage::getPageType()
{
  return _type;
}

char* Xpage::getID()
{
  return _id;
}

char* Xpage::getTitle()
{
  return _title;
}

void Xpage::hide()
{
  _visible = false;
}

void Xpage::show()
{
  _visible = true;
}

XpageKeypad::XpageKeypad(const char* id, const char* title, PageType ptype, const char* target)
  : Xpage(id, title, ptype)
{
  _code = "";
  _mqttTopic = (char*)malloc(strlen(target) + 1);
  strcpy(_mqttTopic, target);
  _trigger = new HADeviceTrigger(HADeviceTrigger::ButtonShortPressType, "AlarmKeypad");
}

void XpageKeypad::clearCode()
{
  _code = "";
}

void XpageKeypad::appendCode(char newkey)
{
  _code += newkey;
}

void XpageKeypad::doOnEnter(ACParmType armtype)
{
  char json[60];
  strcpy(json, "");
  switch (armtype) {
    case ACParmType::disarm:
      sprintf(json, "{\"action\":\"alarm_disarm\", \"code\":\"%s\"}", _code.c_str());
      break;
    case ACParmType::armHome:
      sprintf(json, "{\"action\":\"alarm_arm_home\", \"code\":\"%s\"}", _code.c_str());
      break;
    case ACParmType::armAway:
      sprintf(json, "{\"action\":\"alarm_arm_away\", \"code\":\"%s\"}", _code.c_str());
      break;
    case ACParmType::armNight:
      sprintf(json, "{\"action\":\"alarm_arm_night\", \"code\":\"%s\"}", _code.c_str());
      break;
    case ACParmType::armVacation:
      sprintf(json, "{\"action\":\"alarm_arm_vacation\", \"code\":\"%s\"}", _code.c_str());
      break;
  }
  _trigger->trigger(json);
  SMcontroler.doQuitPage(this);
}

char* XpageKeypad::getTarget()
{
  return _mqttTopic;
}

XpageDialog::XpageDialog(const char* id, const char* title, PageType ptype)
  : Xpage(id, title, ptype)
{
}

void XpageDialog::drawText(JsonArray textlist)
{
  if (_visible)
  {
    Xpage::draw();
    int xpad = 10;
    int ypad = 10;
    lcd.drawRoundRect(_x + xpad, _y + ypad, _w - (xpad * 2), _h - (ypad * 2), displayConfig.cornerradius, COLOR_RED);
    lcd.setFont(&font_M);
    lcd.setTextColor(COLOR_WHITE);
    int ht = lcd.fontHeight() + 2;
    int nbt = 0;
    for (JsonVariant line : textlist)
    {
      lcd.setCursor(_x + xpad * 2, _y + (ypad * 2) + (ht * nbt));
      if (nbt > 0) {
        lcd.setTextColor(COLOR_RED);
        lcd.print(" > ");
        lcd.setTextColor(COLOR_WHITE);
      }
      lcd.println(line.as<const char*>());
      nbt++;
    }
    //DEBUGf("Page %s draw items %d %d %d %d\n", _id, _x, _y, _w, _h);
    for (byte i = 0; i < _nbItems; i++)
    {
      _items[i]->draw(false);
    }
  }
}
