/**
 * Romgere Cockpit Library - https://github.com/romgere/romgere_cockpit
 * @author jerome@mestres.fr
 */


#include "ArduinoRotaryEncoderInterruptControl.h"

/*

Source : http://svglobe.com/arduino/encoders.html


dir. / ret :    <==(-1)==    ==(+1)==>
           |-------------------------------|
    Bit2   | 1 | 1 | 0 | 0 | 1 | 1 | 0 | 0 |
    Bit1   | 0 | 1 | 1 | 0 | 0 | 1 | 1 | 0 |
           |-------------------------------|
    Val    | 2 | 3 | 1 | 0 | 2 | 3 | 1 | 0 |
           |-------------------------------|
Detent :
Type1 Enc : ╔═╗_╔═╗_╔═╗_╔═╗_╔═╗_╔═╗_╔═╗_╔═╗_
Type2 Enc : ╔═╗_____╔═╗_____╔═╗_____╔═╗_____
Type3 Enc : ____╔═╗_____╔═╗_____╔═╗_____╔═╗_
Type4 Enc : ____________╔═╗_____________╔═╗_
                         ▲
                         ║

For all types of rotary encoder, the direction logic is the same. Only detents vary.
Command are send only if rotary is stop on a detent.
*/

#ifdef ACTIVE_MULTI_ARDUINO_BOARD_MODE
ArduinoRotaryEncoderInterruptControl::ArduinoRotaryEncoderInterruptControl(  uint8_t encoderNumber, uint8_t pin1, uint8_t pin2, uint8_t resolution_ppr = 128, uint8_t turns = 4, bool usePullUpPin = false, int boardAddress = -1) : ArduinoInputControl( DigitalControl, ITypeRotaryEncode, (boardAddress != -1), boardAddress){
#else
ArduinoRotaryEncoderInterruptControl::ArduinoRotaryEncoderInterruptControl( uint8_t encoderNumber, uint8_t pin1, uint8_t pin2, uint8_t resolution_ppr = 128, uint8_t turns = 4, bool usePullUpPin = false) : ArduinoInputControl( DigitalControl, ITypeRotaryEncode){
#endif

    this->encoderNumber = encoderNumber;
    this->Pin1 = pin1;
    this->Pin2 = pin2;
    this->maxValue = (resolution_ppr * turns) / 2;
    ArduinoRotaryEncoderInterruptControl::instance0_ = this;

    pinMode(pin1, usePullUpPin? INPUT_PULLUP : INPUT);
    pinMode(pin2, usePullUpPin? INPUT_PULLUP : INPUT);

    this->currentValue = 0; 
    this->lastUpStatus = -1;
    this->isStatusChanged = true;
  
    attachInterrupt(digitalPinToInterrupt(this->Pin1), handleInterruptEncoder0, CHANGE);
    attachInterrupt(digitalPinToInterrupt(this->Pin2), handleInterruptEncoder0, CHANGE);

 /*
  attachInterrupt(digitalPinToInterrupt(this->Pin1), throttleLoop, CHANGE);
  attachInterrupt(digitalPinToInterrupt(this->Pin2), throttleLoop, CHANGE);*/
}


ArduinoRotaryEncoderInterruptControl::~ArduinoRotaryEncoderInterruptControl(){}

// ISR glue routines
void ArduinoRotaryEncoderInterruptControl::handleInterruptEncoder0 ()
{
  instance0_->handleInterrupt ();
}

// for use by ISR glue routines
ArduinoRotaryEncoderInterruptControl * ArduinoRotaryEncoderInterruptControl::instance0_;


// class instance to handle an interrupt
void ArduinoRotaryEncoderInterruptControl::handleInterrupt ()
{
  unsigned char encoderUp;
  unsigned char encoderDn;

  encoderUp = _digitalRead(this->Pin1) == HIGH ? 1 : 0;
  encoderDn = _digitalRead(this->Pin2) == HIGH ? 2 : 0;

  if( encoderUp != this->lastUpStatus ) {
    if( encoderUp ) {
    // rising edge
      if ( encoderDn ) {
        if( this->currentValue < this->maxValue ) this->currentValue ++;
      } else {
        if(this->currentValue > -this->maxValue ) this->currentValue --;
      }
    } else {
    // falling edge
      if ( encoderDn ) {
        if( this->currentValue > -this->maxValue ) this->currentValue --;
      } else {
        if( this->currentValue < this->maxValue ) this->currentValue ++;
      }
    }
  }
  if (this->currentValue > this->maxValue) this->currentValue = this->maxValue;
  if (this->currentValue < -this->maxValue) this->currentValue = -this->maxValue;
  this->lastUpStatus = encoderUp;
  this->isStatusChanged = true;
}


void ArduinoRotaryEncoderInterruptControl::setValue(int16_t value) {
  this->currentValue = value;
}


float ArduinoRotaryEncoderInterruptControl::getValue(){ //returns the current value
  float extValue;
  /*
  Serial.print("currentValue : ");
  Serial.println(this->currentValue);
  Serial.print("extValue : ");
  Serial.println(extValue);
  */
  extValue = ((float)this->currentValue/this->maxValue);

  return extValue;
}

bool ArduinoRotaryEncoderInterruptControl::ReadInput(){ // returns true if there was a change since the last time it was called

   
    Serial.print("Control Debug : Read ArduinoRotaryEncoderInterruptControl[");
    Serial.print(this->Pin1);
    Serial.print(",");
    Serial.print(this->Pin2);
    Serial.print("] Max Value : ");
    Serial.print(this->maxValue);
    Serial.print(" Stat : ");

    Serial.println(this->currentValue);

    // Serial.print(isStatusChanged ? "CHANGE !" : "NO CHANGE");
    // Serial.print(", Value : ");
    // Serial.print(this->LastPinStatus[ROTARY_ENC_OLD_STATUS_INDEX]);
    // Serial.print(",");
    // Serial.print(newVal);
    // Serial.println(".");

    if( this->isStatusChanged)
    {
      this->isStatusChanged = false;
      return true;
    }
    return false;
}
