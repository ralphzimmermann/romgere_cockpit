/**
 * Romgere Cockpit Library - https://github.com/romgere/romgere_cockpit
 * @author jerome@mestres.fr
 */

#ifndef ARDUINOROTARYENCODEINTERRUPTCLASS_H_INCLUDED
#define ARDUINOROTARYENCODEINTERRUPTCLASS_H_INCLUDED

#include <Arduino.h>

#include "../Config/MainConfig.h"
#include "ArduinoControl.h"

#define ROTARY_ENC_CUR_STATUS_INDEX 0
#define ROTARY_ENC_OLD_STATUS_INDEX 1

#define MAX_ENCODERS 2 // lower this value to save memory

// Allows use of rotary encoder
class ArduinoRotaryEncoderInterruptControl : public ArduinoInputControl {

public:
  static void handleInterruptEncoder0();
  static void handleInterruptEncoder1();
  // static void handleInterruptEncoder2 ();
  // static void handleInterruptEncoder3 ();
  static ArduinoRotaryEncoderInterruptControl *instance0_;
  static ArduinoRotaryEncoderInterruptControl *instance1_;
  // static ArduinoRotaryEncoderInterruptControl * instance2_;
  // static ArduinoRotaryEncoderInterruptControl * instance3_;

private:
  volatile uint8_t Pin1;
  volatile uint8_t Pin2;
  volatile uint8_t encoderNumber;
  volatile unsigned char lastUpStatus;
  volatile bool isStatusChanged;
  volatile int currentValue; // ranges from -maxValue to +maxValue with 0 being
                             // the starting point
  int maxValue;
  void handleInterrupt();

public:
#ifdef ACTIVE_MULTI_ARDUINO_BOARD_MODE
  ArduinoRotaryEncoderInterruptControl(uint8_t encoderNumber, uint8_t pin1,
                                       uint8_t pin2,
                                       uint8_t resolution_ppr = 128,
                                       uint8_t turns = 4,
                                       bool usePullUpPin = false,
                                       int boardAddress = -1);
#else
  ArduinoRotaryEncoderInterruptControl(uint8_t encoderNumber, uint8_t pin1,
                                       uint8_t pin2,
                                       uint8_t resolution_ppr = 128,
                                       uint8_t turns = 4,
                                       bool usePullUpPin = false);
#endif
  ~ArduinoRotaryEncoderInterruptControl();
  bool ReadInput();
  float getValue();
  void setValue(int value);
};

#endif // ARDUINOROTARYENCODEINTERRUPTCLASS_H_INCLUDED
