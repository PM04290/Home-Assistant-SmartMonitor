/*

*/
#pragma once

// uncomment line below if screen is under 480x320
//#define SMALL_SCREEN 1

// Choix de la cible
#define SC01
//#define SC01Plus
//#define ESP32_TFT_SPI_480x320
//#define ESP32_TFT_240


//*****************************************************
#ifdef SC01
// describe the pin list available for external using
static uint8_t pinAvailable[] = {0, 5, 25, 26, 35};

#define CORNER_RADIUS 10
class LGFX : public lgfx::LGFX_Device
{
    lgfx::Panel_ST7796  _panel_instance;
    lgfx::Bus_SPI       _bus_instance;
    lgfx::Light_PWM     _light_instance;
    lgfx::Touch_FT5x06  _touch_instance;

  public:
    LGFX(void)
    {
      {
        auto cfg = _bus_instance.config();
        cfg.spi_host = HSPI_HOST;
        cfg.pin_mosi = 13;
        cfg.pin_miso = -1;
        cfg.pin_sclk = 14;
        cfg.pin_dc   = 21;
        cfg.spi_3wire  = true;

        cfg.freq_write = 40000000;
        /*
          cfg.spi_mode = 0;
          cfg.freq_read  = 16000000;
          cfg.use_lock   = true;
        */
        //cfg.dma_channel = SPI_DMA_CH_AUTO;

        _bus_instance.config(cfg);
        _panel_instance.setBus(&_bus_instance);
      }

      {
        auto cfg = _panel_instance.config();

        cfg.pin_cs           =    15;
        cfg.pin_rst          =    22;
        cfg.readable         = false;
        /*
                cfg.pin_busy         =    -1;
                cfg.panel_width      =   320;
                cfg.panel_height     =   480;
                cfg.offset_x         =     0;
                cfg.offset_y         =     0;
                cfg.offset_rotation  =     1;
                cfg.dummy_read_pixel =     8;
                cfg.dummy_read_bits  =     1;
                cfg.invert           = true;
                cfg.rgb_order        = false;
                cfg.dlen_16bit       = false;
                cfg.bus_shared       = true;
        */
        _panel_instance.config(cfg);
      }

      {
        auto cfg = _light_instance.config();

        cfg.pin_bl = 23;
        cfg.invert = false;
        cfg.freq   = 44100;
        cfg.pwm_channel = 7;

        _light_instance.config(cfg);
        _panel_instance.setLight(&_light_instance);
      }

      {
        auto cfg = _touch_instance.config();

        cfg.pin_int    = 39;
        cfg.pin_sda    = 18;
        cfg.pin_scl    = 19;
        cfg.i2c_addr   = 0x38;
        cfg.i2c_port   = 1;
        cfg.freq       = 400000;
        cfg.x_min      = 0;
        cfg.x_max      = 319;
        cfg.y_min      = 0;
        cfg.y_max      = 479;
        cfg.bus_shared = false;
        cfg.offset_rotation = 0;


        _touch_instance.config(cfg);
        _panel_instance.setTouch(&_touch_instance);
      }
      setPanel(&_panel_instance);
    }
};
#endif

//*****************************************************
#ifdef SC01Plus
// describe the pin list available for external using
static uint8_t pinAvailable[] = {10, 11, 12, 13, 14, 21};

#define CORNER_RADIUS 10
class LGFX : public lgfx::LGFX_Device
{
    lgfx::Panel_ST7796  _panel_instance;  // ST7796UI
    lgfx::Bus_Parallel8 _bus_instance;    // MCU8080 8B
    lgfx::Light_PWM     _light_instance;
    lgfx::Touch_FT5x06  _touch_instance;

  public:
    LGFX(void)
    {
      {
        auto cfg = _bus_instance.config();
        cfg.freq_write = 20000000;
        cfg.pin_wr = 47;
        cfg.pin_rd = -1;
        cfg.pin_rs = 0;

        // LCD data interface, 8bit MCU (8080)
        cfg.pin_d0 = 9;
        cfg.pin_d1 = 46;
        cfg.pin_d2 = 3;
        cfg.pin_d3 = 8;
        cfg.pin_d4 = 18;
        cfg.pin_d5 = 17;
        cfg.pin_d6 = 16;
        cfg.pin_d7 = 15;

        _bus_instance.config(cfg);
        _panel_instance.setBus(&_bus_instance);
      }

      {
        auto cfg = _panel_instance.config();

        cfg.pin_cs           =    -1;
        cfg.pin_rst          =    4;
        cfg.pin_busy         =    -1;

        cfg.panel_width      =   320;
        cfg.panel_height     =   480;
        cfg.offset_x         =     0;
        cfg.offset_y         =     0;
        cfg.offset_rotation  =     0;
        cfg.dummy_read_pixel =     8;
        cfg.dummy_read_bits  =     1;
        cfg.readable         =  false;
        cfg.invert           = true;
        cfg.rgb_order        = false;
        cfg.dlen_16bit       = false;
        cfg.bus_shared       =  true;

        _panel_instance.config(cfg);
      }

      {
        auto cfg = _light_instance.config();

        cfg.pin_bl = 45;
        cfg.invert = false;
        cfg.freq   = 44100;
        cfg.pwm_channel = 7;

        _light_instance.config(cfg);
        _panel_instance.setLight(&_light_instance);
      }

      {
        auto cfg = _touch_instance.config();

        cfg.x_min      = 0;
        cfg.x_max      = 319;
        cfg.y_min      = 0;
        cfg.y_max      = 479;
        cfg.pin_int    = 7;
        cfg.bus_shared = true;
        cfg.offset_rotation = 0;

        cfg.i2c_port = 1;
        cfg.i2c_addr = 0x38;
        cfg.pin_sda  = 6;
        cfg.pin_scl  = 5;
        cfg.freq = 400000;

        _touch_instance.config(cfg);
        _panel_instance.setTouch(&_touch_instance);
      }
      setPanel(&_panel_instance);
    }
};
#endif

//*****************************************************
#ifdef ESP32_TFT_SPI_480x320
// describe the pin list available for external using
static uint8_t pinAvailable[] = {25, 26, 36, 39};

#define CORNER_RADIUS 10
class LGFX : public lgfx::LGFX_Device
{
    lgfx::Panel_ST7796  _panel_instance;
    lgfx::Bus_SPI       _bus_instance;
    lgfx::Light_PWM     _light_instance;
    lgfx::Touch_XPT2046 _touch_instance;

  public:
    LGFX(void)
    {
      {
        auto cfg = _bus_instance.config();
        cfg.spi_host = VSPI_HOST;
        cfg.spi_mode = 0;
        cfg.freq_write = 40000000;
        cfg.freq_read  = 16000000;
        cfg.spi_3wire = true;
        cfg.use_lock   = true;
        cfg.pin_sclk = 18;
        cfg.pin_mosi = 23;
        cfg.pin_miso = 19;
        cfg.pin_dc   = 2;

        _bus_instance.config(cfg);
        _panel_instance.setBus(&_bus_instance);
      }

      {
        auto cfg = _panel_instance.config();

        cfg.pin_cs           =    5;
        cfg.pin_rst          =    4;
        cfg.pin_busy         =   -1;

        cfg.panel_width      =   320;
        cfg.panel_height     =   480;
        cfg.offset_x         =     0;
        cfg.offset_y         =     0;
        cfg.offset_rotation  =     0;
        cfg.dummy_read_pixel =     8;
        cfg.dummy_read_bits  =     1;
        cfg.readable         = true;
        cfg.invert           = false;
        cfg.rgb_order        = false;
        cfg.dlen_16bit       = false;

        //cfg.bus_shared       = true;

        _panel_instance.config(cfg);
      }

      {
        auto cfg = _light_instance.config();

        cfg.pin_bl = 16;
        cfg.invert = false;
        cfg.freq   = 44100;
        cfg.pwm_channel = 7;

        _light_instance.config(cfg);
        _panel_instance.setLight(&_light_instance);
      }

      {
        auto cfg = _touch_instance.config();

        // run "calibraeTouch" in controller.init to now values
        cfg.x_min      = 400;
        cfg.x_max      = 3680;
        cfg.y_min      = 3700;
        cfg.y_max      = 280;
        cfg.offset_rotation = 0;

        cfg.pin_int    = 27;

        //cfg.bus_shared = true;

        cfg.spi_host = HSPI_HOST;
        cfg.freq = 1000000;
        cfg.pin_sclk = 14;
        cfg.pin_mosi = 13;
        cfg.pin_miso = 12;
        cfg.pin_cs   = 15;

        _touch_instance.config(cfg);
        _panel_instance.setTouch(&_touch_instance);
      }
      setPanel(&_panel_instance);

    }
};
#endif

//*****************************************************
#ifdef ESP32_TFT_240
// describe the pin list available for external using
static uint8_t pinAvailable[] = {5, 22};

#define CORNER_RADIUS 8
class LGFX : public lgfx::LGFX_Device
{
    lgfx::Panel_ILI9341 _panel_instance;
    lgfx::Bus_Parallel8 _bus_instance;
    lgfx::Touch_XPT2046 _touch_instance;

  public:
    LGFX(void)
    {
      {
        auto cfg = _bus_instance.config();
        cfg.freq_write = 20000000;
        cfg.pin_wr = 4;
        cfg.pin_rd = 2;
        cfg.pin_rs = 15;  // Data / Command

        // LCD data interface, 8bit MCU (8080)
        cfg.pin_d0 = 12;
        cfg.pin_d1 = 13;
        cfg.pin_d2 = 26;
        cfg.pin_d3 = 25;
        cfg.pin_d4 = 17;
        cfg.pin_d5 = 16;
        cfg.pin_d6 = 27;
        cfg.pin_d7 = 14;

        _bus_instance.config(cfg);
        _panel_instance.setBus(&_bus_instance);
      }

      {
        auto cfg = _panel_instance.config();

        cfg.pin_cs           =    33;
        cfg.pin_rst          =    32;
        cfg.pin_busy         =    -1;

        cfg.panel_width      =   240;
        cfg.panel_height     =   320;
        cfg.offset_x         =     0;
        cfg.offset_y         =     0;
        cfg.offset_rotation  =     1;
        cfg.dummy_read_pixel =     8;
        cfg.dummy_read_bits  =     1;
        cfg.readable         =  true;
        cfg.invert           = false;
        cfg.rgb_order        = false;
        cfg.dlen_16bit       = false;
        cfg.bus_shared       =  true;

        _panel_instance.config(cfg);
      }

      {
        auto cfg = _touch_instance.config();

        cfg.x_min      = 0;
        cfg.x_max      = 239;
        cfg.y_min      = 0;
        cfg.y_max      = 319;
        cfg.pin_int    = -1;
        cfg.bus_shared = true;
        cfg.offset_rotation = 0;

        cfg.spi_host = VSPI_HOST;
        cfg.pin_cs   = 22;
        cfg.pin_mosi = 23;
        cfg.pin_miso = 19;
        cfg.pin_sclk = 18;

        cfg.freq = 2700000;

        _touch_instance.config(cfg);
        _panel_instance.setTouch(&_touch_instance);
      }
      setPanel(&_panel_instance);
    }
};
#endif
