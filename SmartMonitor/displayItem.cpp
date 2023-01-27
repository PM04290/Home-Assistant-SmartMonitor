/* https://github.com/lovyan03/LovyanGFX
   https://lang-ship.com/blog/files/LovyanGFX/font/

*/
#include "display.h"
#include "SPIFFS.h"
#include "FS.h"

extern HAMqtt mqtt;
extern Xcontroler SMcontroler;
extern displayConfig_t displayConfig;
extern LGFX lcd;
extern LGFX_Sprite header;
extern LGFX_Sprite zone;
extern LGFX_Sprite kpad;

Xitem::Xitem(uint8_t idx, int32_t x, int32_t y, int32_t w, int32_t h, uint32_t color, const char* title, LGFX_Sprite *sprite)
{
  _idx = idx;
  _page = NULL;
  _x = x; _y = y; _w = w; _h = h;
  _color = color;
  _colorPressed = COLOR_WHITE;
  _targetObj = NULL;
  _sprite = sprite;
  _title = (char*)malloc(strlen(title) + 1);
  strcpy(_title, title);
  _subtype = NULL;
  _action = BtnAction::None;
  _mqtttopic = NULL;
  _mqttstate = NULL;
  _mqttstateattribute = NULL;
  _mqttcommand = NULL;
  _pageID = NULL;
  _dispMode = displayMode::title_text;
  _iconPath = NULL;
  _dataIcon = "";
  _dataUnit = "";
  //_datatype = DataType::implicit;
  _iconSize = 48;
}

Xitem::Xitem(uint8_t idx, int32_t x, int32_t y, int32_t w, int32_t h, uint32_t color, const char* title)
  : Xitem(idx, x, y, w, h, color, title, &zone)
{
}

uint8_t Xitem::getIndex()
{
  return _idx;
}

void Xitem::setPage(Xpage * page) {
  _page = page;
}

Xitem* Xitem::setHAdevice()
{
  char* _subtype = (char*)malloc(strlen(_title) + 1);
  strcpy(_subtype, _title);
  char * pch;
  while (pch = strchr (_subtype, ' ')) {
    *pch = '_';
  }
  _targetObj = new HADeviceTrigger(HADeviceTrigger::ButtonShortPressType, _subtype);
  _action = BtnAction::HAtrigger;
  return this;
}

Xitem* Xitem::setMQTTconfig(const char* target, const char* source, const char* icon, const char* unit)
{
  String str(target);
  String sbase = getValue(str, ';', 0);
  String sstate = getValue(str, ';', 1);
  String scommand = getValue(str, ';', 2);
  if (sbase.length() > 0) {
    _mqtttopic = (char*)malloc(sbase.length() + 1);
    strcpy(_mqtttopic, sbase.c_str());
    _mqttstate = _mqtttopic;
  }
  if (sstate.length() > 0) {
    String sstateAttrib = getValue(sstate, '|', 1);
    if (sstateAttrib != "") {
      sstate = getValue(sstate, '|', 0);
      _mqttstate = (char*)malloc(sbase.length() + sstate.length() + 1);
      strcpy(_mqttstate, sbase.c_str());
      strcat(_mqttstate, sstate.c_str());
      _mqttstateattribute = (char*)malloc(sstateAttrib.length() + 1);
      strcpy(_mqttstateattribute, sstateAttrib.c_str());
    } else {
      _mqttstate = (char*)malloc(sbase.length() + sstate.length() + 1);
      strcpy(_mqttstate, sbase.c_str());
      strcat(_mqttstate, sstate.c_str());
    }
  }
  if (scommand.length() > 0) {
    _mqttcommand = (char*)malloc(sbase.length() + scommand.length() + 1);
    strcpy(_mqttcommand, sbase.c_str());
    strcat(_mqttcommand, scommand.c_str());
  }
  //
  _dataSource = (char*)malloc(strlen(source) + 1);
  strcpy(_dataSource, source);
  _dataIcon = (char*)malloc(strlen(icon) + 1);
  strcpy(_dataIcon, icon);
  _iconPath = new char[50];
  strcpy(_iconPath, "/i/s48-error.png");
  if (strstr(_dataSource, "alarm_control") || strstr(_dataSource, "weather")) {
    _dispMode = displayMode::data_icon;
  }
  if (strstr(_dataSource, "binary_sensor")
      || strstr(_dataSource, "switch")
      || strstr(_dataSource, "cover")
      || strstr(_dataSource, "lock")
      || strstr(_dataSource, "light")) {
    _dispMode = displayMode::data_icon | displayMode::title_text;
  }
  if (_dispMode == displayMode::title_text && strlen(icon)) {
    sprintf(_iconPath, "/i/s48-%s.png", icon);
    _dispMode = displayMode::static_icon | displayMode::data_text;
  }
  _dataUnit = (char*)malloc(strlen(unit) + 1);
  strcpy(_dataUnit, unit);
  return this;
}

Xitem* Xitem::setTargetPage(const char* id)
{
  _pageID = (char*)malloc(strlen(id) + 1);
  strcpy(_pageID, id);
  _action = BtnAction::ChangePage;
  return this;
}

void Xitem::draw(bool pressed)
{
  if (isVisible())
  {
    _sprite->clear();
    _sprite->pushSprite(&lcd, _x, _y);
  }
}

bool Xitem::isTouch(int32_t x, int32_t y)
{
  return isVisible();
}

bool Xitem::doTouch()
{
  DEBUGln("doTouch Base");
  return SMcontroler.resetTempoPage(_page);
}

LGFX_Sprite* Xitem::getSprite()
{
  return _sprite;
}

char* Xitem::getTitle()
{
  return _title;
}

bool Xitem::isSuscribeMQTT()
{
  return _mqttstate != NULL && strlen(_mqttstate) > 0;
}

bool Xitem::isStateAttribute()
{
  return _mqttstateattribute != NULL && strlen(_mqttstateattribute) > 0;
}

char* Xitem::getBaseTopic()
{
  return _mqtttopic;
}

char* Xitem::getStateTopic()
{
  return _mqttstate;
}

char* Xitem::getStateAttribute()
{
  return _mqttstateattribute;
}

char* Xitem::getCommandTopic()
{
  return _mqttcommand;
}

char* Xitem::getTargetPage()
{
  return _pageID;
}

BtnAction Xitem::getAction()
{
  return _action;
}

char* Xitem::getIcon()
{
  return _dataIcon;
}

char* Xitem::getUnit()
{
  return _dataUnit;
}

char* Xitem::getSource()
{
  return _dataSource;
}

int32_t Xitem::x()
{
  return _x;
}

int32_t Xitem::y()
{
  return _y;
}

int32_t Xitem::w()
{
  return _w;
}

int32_t Xitem::h()
{
  return _h;
}

bool Xitem::isVisible()
{
  return _page->isVisible();
}

XitemButton::XitemButton(uint8_t idx, int32_t x, int32_t y, int32_t w, int32_t h, uint32_t color, const char* title)
  : Xitem(idx, x, y, w, h, color, title, &zone)
{
}

void XitemButton::draw(bool pressed)
{
  uint32_t c;
  if (isVisible())
  {
    _sprite->clear();
    _sprite->setFont(&font_L);
    _sprite->setTextSize(1);
    _sprite->setTextDatum(lgfx::middle_center);
    _sprite->drawRoundRect( 7, 7 , _w - 14, _h - 14, displayConfig.cornerradius, pressed ? _colorPressed : COLOR_GRAY);

    _sprite->fillRoundRect( 11, _h - 18 , _w - 22, 8, 4, pressed ? _colorPressed : COLOR_GRAY);

    _sprite->setTextColor(pressed ? _colorPressed : _color);
    _sprite->drawString(_title, _w / 2, 2 * _h / 5);
    _sprite->pushSprite(&lcd, _x, _y);
  }
}

bool XitemButton::isTouch(int32_t x, int32_t y)
{
  int32_t xmax = _x + _w;
  int32_t ymax = _y + _h;
  if (isVisible())
  {
    return ((x >= _x) && (x < xmax) && (y >= _y) && (y < ymax));
  }
  return false;
}

bool XitemButton::doTouch()
{
  Xitem::doTouch();
  DEBUGln("doTouch Button");
  if (_action == BtnAction::HAtrigger && _targetObj != NULL)
  {
    ((HADeviceTrigger*)_targetObj)->trigger("");
  }
  if (_action == BtnAction::ChangePage)
  {
    return SMcontroler.doChangePage(_pageID);
  }
  return true;
}

XitemKPad::XitemKPad(uint8_t idx, int32_t x, int32_t y, int32_t w, int32_t h, uint32_t color, const char* title, ACParmType armType)
  : Xitem(idx, x, y, w, h, color, title, &kpad)
{
  _armType = armType;
  // TODO remplacer l'utilisation de title pour les icon (utiliser iconPath)
  _iconSize = 60;
  switch (_armType)
  {
    case ACParmType::disarm:
      _dispMode = displayMode::static_icon;
      break;
    case ACParmType::armHome:
      _dispMode = displayMode::static_icon;
      break;
    case ACParmType::armAway:
      _dispMode = displayMode::static_icon;
      break;
    case ACParmType::armNight:
      _dispMode = displayMode::static_icon;
      break;
    case ACParmType::armVacation:
      _dispMode = displayMode::static_icon;
      break;
    case ACParmType::armCustom:
      _dispMode = displayMode::static_icon;
      break;
    default:
      _dispMode = displayMode::title_text;
      break;
  }
}

void XitemKPad::draw(bool pressed)
{
  if (isVisible())
  {
    uint32_t c = (pressed ? _colorPressed : _color);
    _sprite->drawRoundRect( 0, 0 , _w, _h, displayConfig.cornerradius, COLOR_GRAY);
    _sprite->fillRoundRect( 1, 1, _w - 2, _h - 2, displayConfig.cornerradius - 1, c);
    if (_dispMode & displayMode::static_icon)
    {
      size_t nb = SMcontroler.readFile(_title);
      if (nb) {
        _sprite->drawPng(SMcontroler.getIconData(), nb, (displayConfig.kpadsize - 60) / 2 - 2, (displayConfig.kpadsize - 60) / 2 - 2);
      }
    }
    if (_dispMode & displayMode::title_text)
    {
      _sprite->setTextColor(pressed ? COLOR_BLACK : COLOR_WHITE);
      _sprite->drawString(_title, _w / 2, _h / 2);
    }
    _sprite->pushSprite(&lcd, _x, _y);
  }
}

bool XitemKPad::isTouch(int32_t x, int32_t y)
{
  int32_t xmax = _x + _w;
  int32_t ymax = _y + _h;
  if (isVisible())
  {
    return ((x >= _x) && (x < xmax) && (y >= _y) && (y < ymax));
  }
  return false;
}

bool XitemKPad::doTouch()
{
  Xitem::doTouch();
  DEBUGln("doTouch Kpad");
  if (_page->getPageType() == PageType::keypadPage)
  {
    if (_armType != ACParmType::none)
    {
      ((XpageKeypad*)_page)->doOnEnter(_armType);
    } else if ((char)_title[0] == '#')
    {
      // none
    } else if ((char)_title[0] == '*')
    {
      ((XpageKeypad*)_page)->clearCode();
    } else
    {
      ((XpageKeypad*)_page)->appendCode((char)_title[0]);
    }
  }
  return true;
}

XitemInfo::XitemInfo(uint8_t idx, int32_t x, int32_t y, int32_t w, int32_t h, uint32_t color, const char* title)
  : Xitem(idx, x, y, w, h, color, title, &zone)
{
  _action = BtnAction::Display;
  _data = "?";
  _nbdata = 1;
}

bool XitemInfo::isTouch(int32_t x, int32_t y)
{
  return false;
}

void XitemInfo::setData(String Sdata)
{
  _data = Sdata;
  if (_dispMode & displayMode::data_icon)
  {
    _data.toLowerCase();
    if (strstr(_dataSource, "alarm_control")) {
      _iconSize = 60;
      sprintf(_iconPath, "/i/d60-acp-%s.png", _data.c_str());
    }
    if (strstr(_dataSource, "weather")) {
      sprintf(_iconPath, "/i/d48-%s.png", _data.c_str());
    }
    if (strstr(_dataSource, "cover") && strstr(_dataSource, "lock")) {
      sprintf(_iconPath, "/i/d48-cover-%s.png", _data.c_str());
    }
    if (strstr(_dataSource, "light")) {
      sprintf(_iconPath, "/i/d48-light-%s.png", _data.c_str());
    }
    if (strstr(_dataSource, "switch")) {
      sprintf(_iconPath, "/i/d48-switch-%s.png", _data.c_str());
    }
    if (strstr(_dataSource, "binary_sensor")) {
      sprintf(_iconPath, "/i/d48-%s-%s.png", _dataIcon, _data.c_str());
    }
  }
  if (isVisible()) {
    draw(false);
  }
}

void XitemInfo::setData(String Sdata, int nbdata)
{
  _nbdata = nbdata;
  XitemInfo::setData(Sdata);
}

void XitemInfo::draw(bool pressed)
{
  _sprite->clear();
  _sprite->setFont(&font_XL);
  if (_nbdata > 2) {
    _sprite->setFont(&font_M);
  }
  if (_nbdata > 4) {
    _sprite->setFont(&font_XS);
  }
  _sprite->setTextSize(1);
  _sprite->setTextColor(_color);

  int32_t xT = _w / 2; // au centre
  int32_t yT = _h / 3;

  int32_t xI = _w / 2 - (_iconSize / 2); // au centre
  int32_t yI = (_h / 2) - (_iconSize / 2) - 1;
  bool iconerror = false;
  size_t nb = 0;
  if (_dispMode & (displayMode::static_icon | displayMode::data_icon))
  {
    nb = SMcontroler.readFile(_iconPath);
    if (nb) {
      if (_dispMode & (displayMode::title_text | displayMode::data_text)) {
        xI = 1; // à gauche car icon + texte
        xT = _iconSize + (_w - _iconSize) / 2; // au centre de ce qu'il reste
      }
      _sprite->drawPng(SMcontroler.getIconData(), nb, xI, yI);
    } else {
      iconerror = true;
    }
  }
  if ((_dispMode & (displayMode::title_text | displayMode::data_text)) || iconerror) {
    if (_action == BtnAction::Command) {
      _sprite->setTextColor(pressed ? COLOR_WHITE : _color);
    }
    if (_dispMode & displayMode::title_text)
    {
      _sprite->setFont(&font_M);
      _sprite->setTextDatum(lgfx::middle_center);
      _sprite->drawString(_title, xT, _h / 2);
    } else
    {
      if (iconerror)
      {
        _sprite->setTextColor(COLOR_RED);
      }
      if (_nbdata > 1)
      {
        _sprite->setTextDatum(lgfx::top_center);
        yT = 4;
        int n = 0;
        String s = "";
        while (n < _nbdata)
        {
          if (n > 0) s += "  ";
          s += getValue(_data, '|', n);
          if (n % 3 == 2 || n == _nbdata - 1)
          {
            _sprite->drawString(s, xT, yT);
            yT += 20;
            s = "";
          }
          n++;
        }
        yT = _h / 3;
      } else
      {
        _sprite->setTextDatum(lgfx::baseline_center);
        _sprite->drawString(_data, xT, yT * 2 - 3);
      }
      _sprite->setFont(&font_XS);
      _sprite->setTextColor(COLOR_GRAY);
      _sprite->setTextDatum(lgfx::top_center);
      _sprite->drawString(String(_title) + " (" + _dataUnit + ")", xT, yT * 2 + 3);
    }
  }

  _sprite->pushSprite(&lcd, _x, _y);
  lcd.display();
}

XitemCommand::XitemCommand(uint8_t idx, int32_t x, int32_t y, int32_t w, int32_t h, uint32_t color, const char* title, BtnAction action)
  : XitemInfo(idx, x, y, w, h, color, title)
{
  _action = action;
}

bool XitemCommand::isTouch(int32_t x, int32_t y)
{
  int32_t xmax = _x + _w;
  int32_t ymax = _y + _h;
  if (isVisible())
  {
    return ((x >= _x) && (x < xmax) && (y >= _y) && (y < ymax));
  }
  return false;
}

bool XitemCommand::doTouch()
{
  Xitem::doTouch();
  if (_action == BtnAction::ChangePage)
  {
    DEBUGln("doTouch changePage");
    return SMcontroler.doChangePage(_pageID);
  }
  DEBUGln("doTouch Command");
  if (_data == "on") {
    return mqtt.publish(_mqttcommand, "OFF");
  }
  if (_data == "off") {
    return mqtt.publish(_mqttcommand, "ON");
  }
  if (_data == "lock") {
    return mqtt.publish(_mqttcommand, "unlock");
  }
  if (_data == "unlock") {
    return mqtt.publish(_mqttcommand, "lock");
  }
  if (_data == "closed") {
    return mqtt.publish(_mqttcommand, "open");
  }
  if (_data == "open") {
    return mqtt.publish(_mqttcommand, "close");
  }
  return false;
}
