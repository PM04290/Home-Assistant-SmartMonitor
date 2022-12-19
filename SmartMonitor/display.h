/* https://github.com/lovyan03/LovyanGFX
   https://lang-ship.com/blog/files/LovyanGFX/font/

*/
#pragma once
#include <Arduino.h>
#include "config.h"
#include "src/fonts-verdana-regular.h"

#define LGFX_USE_V1         // set to use new version of library
#include <LovyanGFX.hpp>    // main library
#include <ArduinoJson.h>
#include <HAintegration.h>
#include "display_setup.h"

static const lgfx::U8g2font font_XXS( myVerdana10_tf );
static const lgfx::U8g2font font_XS( myVerdana14_tf );
static const lgfx::U8g2font font_S( myVerdana18_tf );
static const lgfx::U8g2font font_M( myVerdana22_tf );
static const lgfx::U8g2font font_L( myVerdana28_tf );
static const lgfx::U8g2font font_XL( myRoboto36r_tf );
static const lgfx::U8g2font font_XXL( myRoboto48r_tf );
/*
static const lgfx::U8g2font font_XXS( myRoboto10_tf );
static const lgfx::U8g2font font_XS( myRoboto14_tf );
static const lgfx::U8g2font font_S( myRoboto18_tf );
static const lgfx::U8g2font font_M( myRoboto22_tf );
static const lgfx::U8g2font font_L( myRoboto28_tf );
static const lgfx::U8g2font font_XL( myRoboto36_tf );
static const lgfx::U8g2font font_XXL( myRoboto48_tf );
*/

// http://www.rinkydinkelectronics.com/calc_rgb565.php
#define COLOR_BLACK  0x0000
#define COLOR_WHITE  lcd.color888(255, 255, 255)
#define COLOR_ORANGE lcd.color888(253, 126, 20)
#define COLOR_GREEN  lcd.color888(0, 133, 41)
#define COLOR_GRAY   lcd.color888(80, 80, 80)
#define COLOR_RED    lcd.color888(250, 0, 0)
#define COLOR_CYAN   lcd.color888(0, 210, 210)

struct displayConfig_t {
  uint16_t width;
  uint16_t height;
  uint16_t headerheight;
  uint16_t zonewidth;
  uint16_t zoneheight;
  uint16_t kpadsize;
  uint8_t nbcols;
  uint8_t nbrows;
  uint8_t cornerradius;
};

#define TEMPO_DEFAULT_PAGE 10000
#define TEMPO_DIALOG_PAGE  7000
#define TEMPO_TAP  2000

#define FIFO_MSG_SIZE 64

class Xitem;
class Xpage;
class XpageDialog;

class Xcontroler {
  public:
    explicit Xcontroler();
    void init();
    void drawBackground();
    void drawHeader(char* dateheure);
    void addPage(Xpage* page);
    void loop();
    void setScratchMode();
    void dialog(JsonArray texts);
    int getNbPages();
    Xpage* getPage(int index);
    bool resetTempoPage(Xpage* page);
    bool doChangePage(const char* pageID);
    void doQuitPage(Xpage* page);
    size_t readFile(const char * path);
    uint8_t* getIconData();
    bool screenDump();
    void mqttMsgPush(const char* topic, const char* payload);
    void mqttMsgPop();
    void mqttProcess(mqttMsg* msg);
  protected:
    Xitem* detectTouch();
    Xpage** _listPages;
    uint16_t _nbPages;
    Xpage* _defaultPage;
    XpageDialog* _dlgPage;
    uint32_t _oldTimeDefaultPage;
    // Mqtt messaging
    mqttMsg _msgBuf[FIFO_MSG_SIZE];
    uint16_t _msgWp;
    uint16_t _msgRp;
    uint16_t _msgCount;
};

class Xpage {
  public:
    Xpage(const char* id, const char* title, PageType ptype);
    virtual void draw();
    Xitem* addItem(Xitem* item);
    int nbItems();
    Xitem* getItem(int index);
    Xitem* getItemByIndex(uint8_t index);
    bool isVisible();
    PageType getPageType();
    char* getID();
    char* getTitle();
    void hide();
    void show();
  protected:
    char* _id;
    char* _title;
    int32_t _x;
    int32_t _y;
    int32_t _w;
    int32_t _h;
    bool _visible;
    uint32_t _color;
    Xitem** _items;
    uint16_t _nbItems;
    PageType _type;
};

class XpageKeypad : public Xpage
{
  public:
    XpageKeypad(const char* id, const char* title, PageType ptype, const char* target);
    void clearCode();
    void appendCode(char newkey);
    void doOnEnter(ACParmType armtype);
    char* getTarget();
  private:
    String _code;
    char* _mqttTopic;
    HADeviceTrigger* _trigger;
};

class XpageDialog : public Xpage
{
  public:
    XpageDialog(const char* id, const char* title, PageType ptype);
    void drawText(JsonArray textlist);
  private:
};

class Xitem {
  public:
    Xitem(uint8_t idx, int32_t x, int32_t y, int32_t w, int32_t h, uint32_t color, const char* title, LGFX_Sprite *sprite);
    Xitem(uint8_t idx, int32_t x, int32_t y, int32_t w, int32_t h, uint32_t color, const char* title);
    uint8_t getIndex();
    void setPage(Xpage* page);
    Xitem* setHAdevice();
    Xitem* setTargetPage(const char* page);
    virtual Xitem* setMQTTconfig(const char* target, DataType datatype);
    virtual void draw(bool pressed);
    virtual bool isTouch(int32_t x, int32_t y);
    virtual bool doTouch();
    LGFX_Sprite* getSprite();
    char* getTitle();
    bool isSuscribeMQTT();
    bool isStateAttribute();
    char* getBaseTopic();
    char* getStateTopic();
    char* getStateAttribute();
    char* getCommandTopic();
    char* getTargetPage();
    DataType getDataType();
    BtnAction getAction();
    int32_t x();
    int32_t y();
    int32_t w();
    int32_t h();
    bool isVisible();
  protected:
    uint8_t _idx;
    Xpage* _page;
    int32_t _x;
    int32_t _y;
    int32_t _w;
    int32_t _h;
    char* _title;
    char* _subtype;
    uint32_t _color;
    uint32_t _colorPressed;
    BtnAction _action;
    char* _mqtttopic;
    char* _mqttstate;
    char* _mqttstateattribute;
    char* _mqttcommand;
    char* _pageID;
    void* _targetObj;
    DataType _datatype;
    String _dataUnit;
    LGFX_Sprite* _sprite;
    uint16_t _dispMode;
    char* _iconPath;
};

class XitemButton : public Xitem
{
  public:
    XitemButton(uint8_t idx, int32_t x, int32_t y, int32_t w, int32_t h, uint32_t color, const char* title);
    virtual void draw(bool pressed);
    virtual bool isTouch(int32_t x, int32_t y);
    virtual bool doTouch();
};

class XitemKPad : public Xitem
{
  public:
    XitemKPad(uint8_t idx, int32_t x, int32_t y, int32_t w, int32_t h, uint32_t color, const char* titlext, ACParmType armType);
    virtual void draw(bool pressed);
    virtual bool isTouch(int32_t x, int32_t y);
    virtual bool doTouch();
  private:
    ACParmType _armType;
};

class XitemInfo : public Xitem
{
  public:
    XitemInfo(uint8_t idx, int32_t x, int32_t y, int32_t w, int32_t h, uint32_t color, const char* title);
    virtual void draw(bool pressed);
    virtual bool isTouch(int32_t x, int32_t y);
    void setData(String Sdata);
    void setData(String Sdata, int nbdata);
  protected:
    String _data;
    uint8_t _nbdata;
};

class XitemCommand : public XitemInfo
{
  public:
    XitemCommand(uint8_t idx, int32_t x, int32_t y, int32_t w, int32_t h, uint32_t color, const char* title, BtnAction action);
    virtual bool isTouch(int32_t x, int32_t y);
    virtual bool doTouch();
};

String getValue(String data, char separator, int index);
size_t strends(const char* str1, const char* str2);
