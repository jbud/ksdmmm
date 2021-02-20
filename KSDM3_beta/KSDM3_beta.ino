/**
 * Kia Stinger Drive Mode Memory Module
 * 
 * microcontroller programming in arduino (C++)
 * 
 * by Joe Jackson 2021
 * Version 1.1b
 */

#include <EEPROMW.h>

#define STARTUP_WAIT_TIME 4000
#define WRITE_DELAY       250

// MODES
const byte SMART      = 0x01;
const byte ECO        = 0x02;
const byte COMFORT    = 0x03;  
const byte SPORT      = 0x04;
const byte CUSTOM     = 0x05;

// MASKS
const byte SETUP      = 0x80;
const byte AHOLD      = 0x40;
const byte ISG        = 0x20;
const byte MODE       = 0x07;

const byte KSDEFAULT  = 0xA3; // comfort, isg on, autohold off.

// PINS
const int DMR_IN   = 3;
const int DML_IN   = 4;
const int DMR_OUT  = 5;
const int DML_OUT  = 6;
const int ISG_IN   = 7;
const int ISG_OUT  = 8;
const int AH_IN    = 9;
const int AH_OUT   = 10;

byte currentMode;
bool isSetup, isIsg, isAhold;

byte memByte = 0x00;

bool dm_i = true;
bool ahold_i = true;
bool isg_i = true;
bool changed = false;

EEPROMW eepromw; //eepromw.write(eepromw.read());

byte pack_byte(byte mode, byte isg, byte ahold)
{
  return SETUP | mode | isg | ahold; // setup is implied
}


void unpack_byte(byte b)
{
  isSetup = (b & SETUP) >> 7; // 0xE5 & 0x80 >> 7 = 1
  isAhold = (b & AHOLD) >> 6; // 0xE5 & 0x40 >> 6 = 1
  isIsg = (b & ISG) >> 5;     // 0xE5 & 0x20 >> 5 = 1
  currentMode = b & MODE;     // 0xE5 & 0x07 = 5
}


void setup() 
{
  pinMode(DMR_IN, INPUT_PULLUP);
  pinMode(DML_IN, INPUT_PULLUP);
  pinMode(DMR_OUT, OUTPUT);
  pinMode(DML_OUT, OUTPUT);
  pinMode(ISG_IN, INPUT_PULLUP);
  pinMode(ISG_OUT, OUTPUT);
  pinMode(AH_IN, INPUT);
  pinMode(AH_OUT, OUTPUT);
  eepromw.init();
  memByte = eepromw.read();
  unpack_byte(memByte);

  if (!isSetup) 
  { 
    memByte = KSDEFAULT; 
    eepromw.write(memByte);
    unpack_byte(memByte);    
  }
  delay(STARTUP_WAIT_TIME);

  switch(currentMode){
    case SPORT:
      clockWise(1);
      break;
    case CUSTOM:
      clockWise(2);
      break;
    case SMART:
      counterClockWise(2);
      break;
    case COMFORT:
    case ECO:
    default:
      break;
  }

  if (isAhold){
    digitalWrite(AH_OUT, HIGH);
    delay(WRITE_DELAY);
    digitalWrite(AH_OUT, LOW);
  }
  if (!isIsg){
    digitalWrite(ISG_OUT, HIGH);
    delay(WRITE_DELAY);
    digitalWrite(ISG_OUT, LOW);
  }

}

void loop() 
{
  if (digitalRead(DMR_IN) == HIGH && digitalRead(DML_IN) == HIGH && !dm_i){
    dm_i = true;
  }

  if (digitalRead(ISG_IN) == HIGH && !isg_i){
    isg_i = true;
  }

  if (digitalRead(AH_IN) == LOW && !ahold_i){
    ahold_i = true;
  }

  if (digitalRead(DMR_IN) == LOW && dm_i){
    if (currentMode >= CUSTOM)
    {
      currentMode = CUSTOM; 
    }
    else
    {
      currentMode++;
    }
    changed = true;
    dm_i = false;
  }

  if (digitalRead(DML_IN) == LOW && dm_i){
    if (currentMode <= SMART)
    {
      currentMode = SMART; 
    } 
    else
    {
      currentMode--;
    }
    changed = true;
    dm_i = false;
  }

  if (digitalRead(ISG_IN) == LOW && isg_i){
    isIsg = isIsg;
    changed = true;
    isg_i = false;
  }

  if (digitalRead(AH_IN) == HIGH && ahold_i){
    isAhold = !isAhold;
    changed = true;
    ahold_i = false;
  }

  if (changed)
  {
    memByte = pack_byte(currentMode, isIsg, isAhold);
    eepromw.write(memByte);
    changed = false;
  }

}
void clockWise(int num){
  for(int i=0;i<num;i++){
    digitalWrite(5, HIGH);
    delay(WRITE_DELAY);
    digitalWrite(5, LOW);
    delay(WRITE_DELAY);
  }
}

// simulate a counterclockwise turn of drive mode num times
void counterClockWise(int num){
  for(int i=0;i<num;i++){
    digitalWrite(6, HIGH);
    delay(WRITE_DELAY);
    digitalWrite(6, LOW);
    delay(WRITE_DELAY);
  }
}
