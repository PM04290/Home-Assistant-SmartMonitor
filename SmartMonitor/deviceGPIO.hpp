/*

*/

#define MAX_GPIO 3
int nbGPIO = 0;

extern Xcontroler SMcontroler;

struct GPIOdef {
  HABaseDeviceType* _device;
  int8_t _pin = -1;
  char _id[20] = "";
  char _name[50] = "";
  uint32_t _lastTime = 0;
  float _cA = 1.0;
  float _cB = 0;
  HardwareType _htype;
  bool _oldBinarySensor = false;

  void create(const char* devicename, uint8_t pin, HardwareType htype, const char* name) {
    _pin = pin;
    _lastTime = 0;
    _oldBinarySensor = false;
    strcpy(_name, name);
    _htype = htype;
    //DEBUGf("gpio %s\n",_name);
    switch (htype)
    {
      case HardwareType::buzzer1Pulse:
        SMcontroler.setPinBuzzer(pin, 1);
        //ledcAttachPin(pin, 0);
        pinMode(pin, OUTPUT);
        break;
      case HardwareType::buzzerPWM:
        SMcontroler.setPinBuzzer(pin, 2);
        ledcAttachPin(pin, 0); // canal 0
        break;
      case HardwareType::SoundI2S:
        SMcontroler.setPinBuzzer(pin, 3);
        break;
      case HardwareType::luminositySensor:
        SMcontroler.setLuminosityPin(pin);
        break;
      case HardwareType::binarySensor:
        pinMode(pin, INPUT_PULLUP);
        sprintf(_id, "%s_bs%d", devicename, nbGPIO);
        _device = new HABinarySensor(_id);
        _device->setName(_name);
        _oldBinarySensor = digitalRead(pin);
        ((HABinarySensor*)_device)->setCurrentState(_oldBinarySensor);
        //((HABinarySensor*)_device)->setDeviceClass("door"); // TODO
        break;
      case HardwareType::numberSensor:
        sprintf(_id, "%s_ns%d", devicename, nbGPIO);
        _device = new HASensorNumber(_id, HASensorNumber::PrecisionP1);
        _device->setName(_name);
        break;
    }
  }

  void setCoef(float A, float B)
  {
    _cA = A;
    _cB = B;
    DEBUGf("coef A %f B %f\n", A, B);
  }

  void process(bool topSecond) {
    uint32_t t = millis();
    if (_htype == HardwareType::binarySensor)
    {
      bool sensor = digitalRead(_pin);
      if (_oldBinarySensor != sensor)
      {
        if (_lastTime == 0)
        {
          _lastTime = t;
        } else
        {
          if (t < _lastTime || (t > _lastTime + 50))
          {
            _oldBinarySensor = sensor;
            ((HABinarySensor*)_device)->setState(_oldBinarySensor, true);
            //DEBUGf("contact[%d] = %d\n", _pin, _oldBinarySensor);
            _lastTime = 0;
          }
        }
      }
      else
      {
        _lastTime = 0;
      }
    }
    if (topSecond && (_htype == HardwareType::numberSensor))
    {
      uint32_t sum = 0;
      for (int i = 0; i < 20; i++)
      {
        sum += analogRead(_pin);
      }
      sum = sum / 20;
      float v = _cA * (float)sum + _cB;
      ((HASensorNumber*)_device)->setValue(v, false);
    }
  }
};

GPIOdef GPIOs[MAX_GPIO];
