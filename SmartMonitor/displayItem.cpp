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
  _dataUnit = "";
  _datatype = DataType::implicit;
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

Xitem* Xitem::setMQTTconfig(const char* target, DataType datatype)
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
  _datatype = datatype;
  _iconPath = new char[50];
  switch (_datatype)
  {
    case DataType::alarm_state:
    case DataType::weather_state:
      strcpy(_iconPath, "/i/s48-error.png");
      _dispMode = displayMode::data_icon;
      break;
    case DataType::cover_state:
    case DataType::light_state:
    case DataType::lock_state:
    case DataType::switch_state:
    case DataType::binary_state:
      strcpy(_iconPath, "/i/s48-error.png");
      _dispMode = displayMode::data_icon | displayMode::title_text;
      break;
    case DataType::apparent_power:
      strcpy(_iconPath, "/i/s48-power.png");
      _dispMode = displayMode::static_icon | displayMode::data_text;
      _dataUnit = "VA";
      break;
    case DataType::aqi:
      strcpy(_iconPath, "/i/s48-aqi.png");
      _dispMode = displayMode::static_icon | displayMode::data_text;
      break;
    case DataType::atmospheric_pressure:
      strcpy(_iconPath, "/i/s48-pressure.png");
      _dispMode = displayMode::static_icon | displayMode::data_text;
      _dataUnit = "hPa";
      break;
    case DataType::battery:
      strcpy(_iconPath, "/i/s48-battery.png");
      _dispMode = displayMode::static_icon | displayMode::data_text;
      _dataUnit = "%";
      break;
    case DataType::carbon_dioxide:
      strcpy(_iconPath, "/i/s48-co2.png");
      _dispMode = displayMode::static_icon | displayMode::data_text;
      _dataUnit = "ppm";
      break;
    case DataType::carbon_monoxide:
      strcpy(_iconPath, "/i/s48-co.png");
      _dispMode = displayMode::static_icon | displayMode::data_text;
      _dataUnit = "ppm";
      break;
    case DataType::current:
      strcpy(_iconPath, "/i/s48-current.png");
      _dispMode = displayMode::static_icon | displayMode::data_text;
      _dataUnit = "A";
      break;
    case DataType::distance:
      strcpy(_iconPath, "/i/s48-distance.png");
      _dispMode = displayMode::static_icon | displayMode::data_text;
      _dataUnit = "m";
      break;
    case DataType::duration:
      strcpy(_iconPath, "/i/s48-duration.png");
      _dispMode = displayMode::static_icon | displayMode::data_text;
      _dataUnit = "s";
      break;
    case DataType::energy:
      strcpy(_iconPath, "/i/s48-energy.png");
      _dispMode = displayMode::static_icon | displayMode::data_text;
      _dataUnit = "kWh";
      break;
    case DataType::frequency:
      strcpy(_iconPath, "/i/s48-frequency.png");
      _dispMode = displayMode::static_icon | displayMode::data_text;
      _dataUnit = "Hz";
      break;
    case DataType::gas:
      strcpy(_iconPath, "/i/s48-gas.png");
      _dispMode = displayMode::static_icon | displayMode::data_text;
      _dataUnit = "m³";
      break;
    case DataType::humidity:
      strcpy(_iconPath, "/i/s48-humidity.png");
      _dispMode = displayMode::static_icon | displayMode::data_text;
      _dataUnit = "%";
      break;
    case DataType::illuminance:
      strcpy(_iconPath, "/i/s48-illuminance.png");
      _dispMode = displayMode::static_icon | displayMode::data_text;
      _dataUnit = "lx";
      break;
    case DataType::irradiance:
      strcpy(_iconPath, "/i/s48-irradiance.png");
      _dispMode = displayMode::static_icon | displayMode::data_text;
      _dataUnit = "W/m²";
      break;
    case DataType::moisture:
      strcpy(_iconPath, "/i/s48-humidity.png");
      _dispMode = displayMode::static_icon | displayMode::data_text;
      _dataUnit = "%";
      break;
    case DataType::monetary:
      strcpy(_iconPath, "/i/s48-monetary.png");
      _dispMode = displayMode::static_icon | displayMode::data_text;
      _dataUnit = "";
      break;
    case DataType::nitrogen_dioxide:
      strcpy(_iconPath, "/i/s48-no2.png");
      _dispMode = displayMode::static_icon | displayMode::data_text;
      _dataUnit = "µg/m³";
      break;
    case DataType::nitrogen_monoxide:
      strcpy(_iconPath, "/i/s48-no.png");
      _dispMode = displayMode::static_icon | displayMode::data_text;
      _dataUnit = "µg/m³";
      break;
    case DataType::nitrous_oxide:
      strcpy(_iconPath, "/i/s48-n2o.png");
      _dispMode = displayMode::static_icon | displayMode::data_text;
      _dataUnit = "µg/m³";
      break;
    case DataType::ozone:
      strcpy(_iconPath, "/i/s48-o3.png");
      _dispMode = displayMode::static_icon | displayMode::data_text;
      _dataUnit = "µg/m³";
      break;
    case DataType::pm1:
    case DataType::pm25:
    case DataType::pm10:
      strcpy(_iconPath, "/i/s48-pm.png");
      _dispMode = displayMode::static_icon | displayMode::data_text;
      _dataUnit = "µg/m³";
      break;
    case DataType::power:
      strcpy(_iconPath, "/i/s48-power.png");
      _dispMode = displayMode::static_icon | displayMode::data_text;
      _dataUnit = "W";
      break;
    case DataType::power_factor:
      strcpy(_iconPath, "/i/s48-power-factor.png");
      _dispMode = displayMode::static_icon | displayMode::data_text;
      _dataUnit = "%";
      break;
    case DataType::precipitation:
      strcpy(_iconPath, "/i/s48-precipitation.png");
      _dispMode = displayMode::static_icon | displayMode::data_text;
      _dataUnit = "mm";
      break;
    case DataType::precipitation_intensity:
      strcpy(_iconPath, "/i/s48-precipitation.png");
      _dispMode = displayMode::static_icon | displayMode::data_text;
      _dataUnit = "mm/j";
      break;
    case DataType::pressure:
      strcpy(_iconPath, "/i/s48-pressure.png");
      _dispMode = displayMode::static_icon | displayMode::data_text;
      _dataUnit = "hPa";
      break;
    case DataType::reactive_power:
      strcpy(_iconPath, "/i/s48-power.png");
      _dispMode = displayMode::static_icon | displayMode::data_text;
      _dataUnit = "var";
      break;
    case DataType::signal_strength:
      strcpy(_iconPath, "/i/s48-signal.png");
      _dispMode = displayMode::static_icon | displayMode::data_text;
      _dataUnit = "dBm";
      break;
    case DataType::sound_pressure:
      strcpy(_iconPath, "/i/s48-sound.png");
      _dispMode = displayMode::static_icon | displayMode::data_text;
      _dataUnit = "dBA";
      break;
    case DataType::speed:
      strcpy(_iconPath, "/i/s48-speed.png");
      _dispMode = displayMode::static_icon | displayMode::data_text;
      _dataUnit = "Km/h";
      break;
    case DataType::sulphur_dioxide:
      strcpy(_iconPath, "/i/s48-so2.png");
      _dispMode = displayMode::static_icon | displayMode::data_text;
      _dataUnit = "µg/m³";
      break;
    case DataType::temperature:
      strcpy(_iconPath, "/i/s48-temperature.png");
      _dispMode = displayMode::static_icon | displayMode::data_text;
      _dataUnit = "°";
      break;
    case DataType::volatile_organic_compounds:
      strcpy(_iconPath, "/i/s48-cov.png");
      _dispMode = displayMode::static_icon | displayMode::data_text;
      _dataUnit = "µg/m³";
      break;
    case DataType::voltage:
      strcpy(_iconPath, "/i/s48-voltage.png");
      _dispMode = displayMode::static_icon | displayMode::data_text;
      _dataUnit = "V";
      break;
    case DataType::volume:
    case DataType::water:
      strcpy(_iconPath, "/i/s48-volume.png");
      _dispMode = displayMode::static_icon | displayMode::data_text;
      _dataUnit = "L";
      break;
    case DataType::weight:
      strcpy(_iconPath, "/i/s48-weight.png");
      _dispMode = displayMode::static_icon | displayMode::data_text;
      _dataUnit = "Kg";
      break;
    case DataType::wind_speed:
      strcpy(_iconPath, "/i/s48-wind-speed.png");
      _dispMode = displayMode::static_icon | displayMode::data_text;
      _dataUnit = "Km/h";
      break;
    case DataType::elevation:
      strcpy(_iconPath, "/i/s48-elevation.png");
      _dispMode = displayMode::static_icon | displayMode::data_text;
      _dataUnit = "°";
      break;
    default:
      strcpy(_iconPath, "/i/s48-error.png");
      _dispMode = displayMode::title_text;
      break;
  }
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

DataType Xitem::getDataType()
{
  return _datatype;
}

BtnAction Xitem::getAction()
{
  return _action;
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
    _sprite->setTextColor(pressed ? _colorPressed : _color);
    _sprite->drawString(_title, _w / 2, _h / 2);
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
        _sprite->drawPng(SMcontroler.getIconData(), nb, (displayConfig.kpadsize - 48) / 2 - 2, (displayConfig.kpadsize - 48) / 2 - 2);
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
    switch (_datatype)
    {
      case DataType::alarm_state:
        sprintf(_iconPath, "/i/d48-%s.png", _data.c_str());
        break;
      case DataType::weather_state:
        _data.toLowerCase();
        sprintf(_iconPath, "/i/d48-%s.png", _data.c_str());
        break;
      case DataType::cover_state:
        _data.toLowerCase();
        sprintf(_iconPath, "/i/d48-cover-%s.png", _data.c_str());
        break;
      case DataType::light_state:
        _data.toLowerCase();
        sprintf(_iconPath, "/i/d48-light-%s.png", _data.c_str());
        break;
      case DataType::lock_state:
        _data.toLowerCase();
        sprintf(_iconPath, "/i/d48-lock-%s.png", _data.c_str());
        break;
      case DataType::switch_state:
        _data.toLowerCase();
        sprintf(_iconPath, "/i/d48-switch-%s.png", _data.c_str());
        break;
      case DataType::binary_state:
        _data.toLowerCase();
        sprintf(_iconPath, "/i/d48-%s.png", _data.c_str());
        break;
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
  //_sprite->drawRoundRect( 0, 0 , _w, _h, displayConfig.cornerradius, COLOR_GRAY);

  // affichage normal avec image et text
  int Isize = 48;

  int32_t xT = _w / 2; // au centre
  int32_t yT = _h / 3;

  int32_t xI = _w / 2 - (Isize / 2); // au centre
  int32_t yI = (_h / 2) - (Isize / 2) - 1;
  bool iconerror = false;
  size_t nb = 0;
  if (_dispMode & (displayMode::static_icon | displayMode::data_icon))
  {
    nb = SMcontroler.readFile(_iconPath);
    if (nb) {
      if (_dispMode & (displayMode::title_text | displayMode::data_text)) {
        xI = 1; // à gauche car icon + texte
        xT = Isize + (_w - Isize) / 2; // au centre de ce qu'il reste
      }
      _sprite->drawPng(SMcontroler.getIconData(), nb, xI, yI);
    } else {
      iconerror = true;
    }
  }
  if ((_dispMode & (displayMode::title_text | displayMode::data_text)) || iconerror) {
    if (_action == BtnAction::Command) {
      _sprite->setFont(&font_M);
      _sprite->setTextDatum(lgfx::middle_center);
      _sprite->setTextColor(pressed ? COLOR_WHITE : _color);
    }
    if (_dispMode & displayMode::title_text)
    {
      _sprite->setFont(&font_L);
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
        _sprite->drawString(_data + _dataUnit, xT, yT * 2 - 3);
      }
      _sprite->setFont(&font_XS);
      _sprite->setTextColor(COLOR_GRAY);
      _sprite->setTextDatum(lgfx::top_center);
      _sprite->drawString(_title, xT, yT * 2 + 3);
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
  switch (_datatype) {
    case DataType::switch_state:
    case DataType::light_state:
      if (_data == "on") {
        return mqtt.publish(_mqttcommand, "OFF");
      }
      if (_data == "off") {
        return mqtt.publish(_mqttcommand, "ON");
      }
      break;
    case DataType::lock_state:
      if (_data == "lock") {
        return mqtt.publish(_mqttcommand, "unlock");
      }
      if (_data == "unlock") {
        return mqtt.publish(_mqttcommand, "lock");
      }
      break;
    case DataType::cover_state:
      if (_data == "closed") {
        return mqtt.publish(_mqttcommand, "open");
      }
      if (_data == "open") {
        return mqtt.publish(_mqttcommand, "close");
      }
      break;
  }
  return false;
}
