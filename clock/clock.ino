#include <LiquidCrystal_I2C.h>
#include <Wire.h>

#define TOGGLE_SWITCH 2

namespace lcd {

  enum request {
    off, on, toggle, none
  };

  LiquidCrystal_I2C* display;
  constexpr uint8_t width = 16, height = 2;
  bool state = HIGH;
  request stateChangeRequest = request::none;

  uint8_t getFirstI2CAddress() {
    Wire.begin();
    while (true) {
      for (uint8_t address = 1; address < 127; ++address) {
        Wire.beginTransmission(address);
        if (Wire.endTransmission() == 0) return address;
      }
      for (uint8_t i = 0, state = LOW; i < 8; i++, state = !state) {
        digitalWrite(LED_BUILTIN, state = !state);
        delay(250);
      }
    }
  }

  void print(uint8_t line, const String& text) {
    display->setCursor(0, line);
    for (uint8_t i = 0; i < (width - text.length()) / 2; ++i) display->print(' ');
    display->print(text);
    for (uint8_t i = 0; i < (width - text.length() + 1) / 2; ++i) display->print(' ');
  }

  void handleStateChangeRequest() {
    if (stateChangeRequest != request::none) {
      if (stateChangeRequest == request::toggle) state = !state;
      else if (stateChangeRequest == request::on) state = HIGH;
      else if (stateChangeRequest == request::off) state = LOW;
      stateChangeRequest = request::none;
      state ? display->display() : display->noDisplay();
      state ? display->backlight() : display->noBacklight();
    }
  }

  void begin() {
    display = new LiquidCrystal_I2C(getFirstI2CAddress(), width, height);
    display->init();
    display->backlight();
  }
}

void updateTime(const String& time) {
//  const uint8_t hour = time.substring(0, 2).toInt();
//  const uint8_t minute = time.substring(3, 5).toInt();
//  const bool amPm = time[5] == 'P'; // AM = false, PM = true
  lcd::print(0, time);
}

void toggle() {
  lcd::stateChangeRequest = lcd::request::toggle;
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(TOGGLE_SWITCH, INPUT);
  attachInterrupt(digitalPinToInterrupt(TOGGLE_SWITCH), toggle, FALLING);
  lcd::begin();
  Serial.begin(9600);
}

void loop() {
  const String data = Serial.readStringUntil('\n');
  if (data.length() > 0) {
    if (data[0] == 't') updateTime(data.substring(1));
    else if (data[0] == 's') lcd::stateChangeRequest = data[1] == '1' ? lcd::request::on : lcd::request::off;
    Serial.println(String("OK") + (lcd::state ? '1' : '0') + '\n');
  }
  lcd::handleStateChangeRequest();
}
