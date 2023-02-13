/**
 * Romgere Cockpit Library - https://github.com/romgere/romgere_cockpit
 * @author jerome@mestres.fr
 */

#include "ArduinoRotaryEncoderInterruptControl.h"

#ifdef ACTIVE_MULTI_ARDUINO_BOARD_MODE
ArduinoRotaryEncoderInterruptControl::ArduinoRotaryEncoderInterruptControl(
    uint8_t encoderNumber, uint8_t pin1, uint8_t pin2,
    uint8_t resolution_ppr = 128, uint8_t turns = 4, bool usePullUpPin = false,
    int boardAddress = -1)
    : ArduinoInputControl(DigitalControl, ITypeRotaryEncode,
                          (boardAddress != -1), boardAddress) {
#else
ArduinoRotaryEncoderInterruptControl::ArduinoRotaryEncoderInterruptControl(
    uint8_t encoderNumber, uint8_t pin1, uint8_t pin2,
    uint8_t resolution_ppr = 128, uint8_t turns = 4, bool usePullUpPin = false)
    : ArduinoInputControl(DigitalControl, ITypeRotaryEncode) {
#endif

  this->encoderNumber = encoderNumber;
  this->Pin1 = pin1;
  this->Pin2 = pin2;
  this->maxValue = ((int)resolution_ppr * (int)turns);

  pinMode(pin1, usePullUpPin ? INPUT_PULLUP : INPUT);
  pinMode(pin2, usePullUpPin ? INPUT_PULLUP : INPUT);

  this->currentValue = this->maxValue / 2;
  this->lastUpStatus = -1;
  this->isStatusChanged = true;

  switch (encoderNumber) {
  case 0:
    ArduinoRotaryEncoderInterruptControl::instance0_ = this;
    attachInterrupt(digitalPinToInterrupt(this->Pin1), handleInterruptEncoder0,
                    CHANGE);
    attachInterrupt(digitalPinToInterrupt(this->Pin2), handleInterruptEncoder0,
                    CHANGE);
    break;
  case 1:
    ArduinoRotaryEncoderInterruptControl::instance1_ = this;
    attachInterrupt(digitalPinToInterrupt(this->Pin1), handleInterruptEncoder1,
                    CHANGE);
    attachInterrupt(digitalPinToInterrupt(this->Pin2), handleInterruptEncoder1,
                    CHANGE);
    break;
  }
}

ArduinoRotaryEncoderInterruptControl::~ArduinoRotaryEncoderInterruptControl() {}

// ISR glue routines
void ArduinoRotaryEncoderInterruptControl::handleInterruptEncoder0() {
  instance0_->handleInterrupt();
}

void ArduinoRotaryEncoderInterruptControl::handleInterruptEncoder1() {
  instance1_->handleInterrupt();
}

// for use by ISR glue routines
ArduinoRotaryEncoderInterruptControl
    *ArduinoRotaryEncoderInterruptControl::instance0_;
ArduinoRotaryEncoderInterruptControl
    *ArduinoRotaryEncoderInterruptControl::instance1_;

// class instance to handle interrupt
void ArduinoRotaryEncoderInterruptControl::handleInterrupt() {
  uint8_t newVal = 0;
  newVal += _digitalRead(this->Pin1) == HIGH ? 1 : 0;
  newVal += _digitalRead(this->Pin2) == HIGH ? 2 : 0;

  if (newVal & 1 != this->lastUpStatus) {
    switch (newVal) {
    case 0: // falling edge + falling edge
      if (this->currentValue < this->maxValue)
        this->currentValue++;
      break;
    case 1: // rising edge + falling edge
      if (this->currentValue > 0)
        this->currentValue--;
      ;
      break;
    case 2: // falling edge + rising edge
      if (this->currentValue > 0)
        this->currentValue--;
      break;
    case 3: // rising edge + rising edge
      if (this->currentValue < this->maxValue)
        this->currentValue++;
      break;
    }
  }

  this->lastUpStatus = newVal & 1;
  this->isStatusChanged =
      true; // if the interrupt got triggered, something has changed
}

void ArduinoRotaryEncoderInterruptControl::setValue(int16_t value) {
  this->currentValue = value;
}

float ArduinoRotaryEncoderInterruptControl::getValue() { // returns the current
                                                         // value
  float extValue;
  extValue = ((float)this->currentValue / this->maxValue);
  return extValue;
}

bool ArduinoRotaryEncoderInterruptControl::
    ReadInput() { // returns true if there was a change since the last time it
                  // was called

  /*
    Serial.print("Control Debug : Read ArduinoRotaryEncoderInterruptControl[");
    Serial.print(this->Pin1);
    Serial.print(",");
    Serial.print(this->Pin2);
    Serial.print("] Max Value : ");
    Serial.print(this->maxValue);
    Serial.print(" Stat : ");

    Serial.println(this->currentValue);
  */

  if (this->isStatusChanged) {
    this->isStatusChanged = false;
    return true;
  }
  return false;
}
