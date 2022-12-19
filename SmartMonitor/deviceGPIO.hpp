/*

*/

#define MAX_GPIO 3
int nbGPIO = 0;

struct GPIOdef {
  HABaseDeviceType* _device;
  uint8_t _pin = 0;
  char _id[20] = "";
  char _name[50] = "";
  uint32_t _lastTime = 0;
  HardwareType _htype;
  bool _oldBinarySensor = false;

  void create(const char* devicename, uint8_t pin, HardwareType htype, const char* name) {
    _pin = pin;
    _lastTime = 0;
    _oldBinarySensor = false;
    pinMode(pin, INPUT_PULLUP);
    strcpy(_name, name);
    //DEBUGf("gpio %s\n",_name);
    switch (htype)
    {
      case HardwareType::binarySensor:
        sprintf(_id, "%s_bs%d", devicename, nbGPIO);
        _device = new HABinarySensor(_id);
        _device->setName(_name);
        _oldBinarySensor = digitalRead(pin);
        ((HABinarySensor*)_device)->setCurrentState(_oldBinarySensor);
        ((HABinarySensor*)_device)->setDeviceClass("door"); // TODO
        break;
    }
  }

  void process() {
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
  }
};
GPIOdef GPIOs[MAX_GPIO];
