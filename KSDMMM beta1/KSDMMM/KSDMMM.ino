/**
 * Kia Stinger Drive Mode Memory Module
 * 
 * microcontroller programming in arduino (C++)
 * 
 * by Joe Jackson 2020
 * Version 1.0.1b
 */
#include <EEPROM.h>

#include <Wire.h>
#include <FRAM.h>

FRAM fram;

// comment this out when deployed:
#define DEBUG

// beta stuff (uncomment to use beta features)
#define BETA


// some arduinos have issues with pulling ground to some pins upon startup.
#define ERROR_OFFSET 1

// Set time to wait before outputing memory to vehicle in ms (recommended 2000-5000)
#define STARTUP_WAIT_TIME 4000

#ifdef DEBUG
 #define DEBUG_PRINT(x)  Serial.println (x)
#else
 #define DEBUG_PRINT(x)
#endif

int memDAddr = 0; 
int memSAddr = 1;
int memFuture1Addr = 2;
int memFuture2Addr = 3;

const byte SMARTMODE = 0;
const byte ECOMODE = 1;
const byte COMFORTMODE = 2;
const byte SPORTMODE = 3;
const byte CUSTOMMODE = 4;

byte currentMode;

bool dm_inputAllowed = true;

#ifdef BETA
bool ahold = false;
bool ahold_inputAllowed = true;
#endif
bool isg = true;
bool isg_inputAllowed = true;



void setup() {
  pinMode(3, INPUT_PULLUP); // clockwise input
  pinMode(4, INPUT_PULLUP); // counterclockwise input
  pinMode(5, OUTPUT);
  pinMode(6, OUTPUT);
  pinMode(7, INPUT_PULLUP); // ISG (Auto Start Stop) input
  pinMode(8, OUTPUT);
  pinMode(9, INPUT_PULLUP); // Auto Hold input
  pinMode(10, OUTPUT);
#ifndef BETA
  if (EEPROM.read(memFuture2Addr)) {
    ahold = true; // enable Autohold
  }

  if (!EEPROM.read(memFuture1Addr)) {
    isg = false; // disable ISG
  }
  //If initial setup hasn't completed, set the default drive mode
  if (EEPROM.read(memSAddr) != 1) { 
    // Write the default to permanent memory
    EEPROM.write(memDAddr, COMFORTMODE);
    EEPROM.write(memSAddr, 1);
    currentMode = COMFORTMODE;
  } else {
    // Setup has completed read the current mode from permanent memory
    currentMode = EEPROM.read(memDAddr); 
  }
#else
  if (memRead(memFuture2Addr)) {
    ahold = true; // enable Autohold
  }

  if (!memRead(memFuture1Addr)) {
    isg = false; // disable ISG
  }
  //If initial setup hasn't completed, set the default drive mode
  if (memRead(memSAddr) != 1) { 
    // Write the default to permanent memory
    memWrite(memDAddr, COMFORTMODE);
    memWrite(memSAddr, 1);
    currentMode = COMFORTMODE;
  } else {
    // Setup has completed read the current mode from permanent memory
    currentMode = memRead(memDAddr); 
  }
#endif

#ifdef DEBUG
  Serial.begin(19200);
#endif

  bool modechanged = true;
  delay(STARTUP_WAIT_TIME);
  switch(currentMode){
      case SPORTMODE:
        clockWise(1-ERROR_OFFSET);
        break;
      case CUSTOMMODE:
        clockWise(2-ERROR_OFFSET);
        break;
      case SMARTMODE:
        counterClockWise(2+ERROR_OFFSET);
        break;
      case COMFORTMODE:
        counterClockWise(ERROR_OFFSET);
        break;
      case ECOMODE:
        counterClockWise(ERROR_OFFSET);
        break;
      default:
        modechanged = false;
        break;
  }
  if (modechanged){
    DEBUG_PRINT("mode has changed.");
  }
#ifdef BETA
  if (ahold){
    digitalWrite(10, HIGH);
    delay(250);
    digitalWrite(10, LOW);
    DEBUG_PRINT("Autohold Enabled");
  }
#endif
  // if ISG is disabled in memory, send a signal. ISG is on by default.
  if (!isg){
    digitalWrite(8, HIGH);
    delay(250);
    digitalWrite(8, LOW);
    DEBUG_PRINT("ISG Disabled");
  }
  char s[50];
  sprintf(s, "setup mode: %s", modeText(currentMode));
  DEBUG_PRINT(s);
}

void loop() {
  char c[50];
  // constantly read the input pins increment/decrement 
  // the drive mode and store it in permanent memory
  if (digitalRead(4) == HIGH && digitalRead(3) == HIGH && !dm_inputAllowed){
    dm_inputAllowed = true;
  }
  
  if (digitalRead(4) == LOW && dm_inputAllowed){
    if (currentMode >= CUSTOMMODE){
      currentMode = CUSTOMMODE; 
    } else {
      currentMode++;
    }
    sprintf(c, "Clock Triggered, Current Mode: %s", modeText(currentMode));
#ifndef BETA
    EEPROM.write(memDAddr, currentMode);
#else
    memWrite(memDAddr, currentMode);
#endif
    DEBUG_PRINT(c);
    dm_inputAllowed = false;
  }
  if (digitalRead(3) == LOW && dm_inputAllowed){
    if (currentMode <= SMARTMODE){
      currentMode = SMARTMODE; 
    } else {
      currentMode--;
    }
    sprintf(c, "CClock Triggered, Current Mode: %s", modeText(currentMode));
#ifndef BETA
    EEPROM.write(memDAddr, currentMode);
#else
    memWrite(memDAddr, currentMode);
#endif
    DEBUG_PRINT(c);
    dm_inputAllowed = false;
  }
#ifdef BETA
  if (digitalRead(9) == HIGH && !ahold_inputAllowed){
    ahold_inputAllowed = true;
  }
  if (digitalRead(9) == LOW && ahold_inputAllowed){
    ahold = !ahold; // reverse the value from current.
    sprintf(c, "Auto Hold Triggered, Current Mode: %s", ahold ? "on" : "off");

#ifndef BETA
    EEPROM.write(memFuture2Addr, ahold);
#else
    memWrite(memFuture2Addr, ahold);
#endif
    
    DEBUG_PRINT(c);
    ahold_inputAllowed = false;
  }
#endif
  if (digitalRead(7) == HIGH && !isg_inputAllowed){
    isg_inputAllowed = true;
  }
  if (digitalRead(7) == LOW && isg_inputAllowed){
    isg = !isg; // reverse the value from current.
    sprintf(c, "ISG Triggered, Current Mode: %s", isg ? "on" : "off");
#ifndef BETA
    EEPROM.write(memFuture1Addr, isg);
#else
    memWrite(memFuture1Addr, isg);
#endif
    DEBUG_PRINT(c);
    isg_inputAllowed = false;
  }
}

// simulate a clockwise turn of drive mode num times
void clockWise(int num){
  for(int i=0;i<num;i++){
    digitalWrite(5, HIGH);
    delay(250);
    digitalWrite(5, LOW);
    delay(250);
    DEBUG_PRINT("clockset");
  }
}

// simulate a counterclockwise turn of drive mode num times
void counterClockWise(int num){
  for(int i=0;i<num;i++){
    digitalWrite(6, HIGH);
    delay(250);
    digitalWrite(6, LOW);
    delay(250);
    DEBUG_PRINT("cclockset");
  }
}

void memWrite(int addr, int val){
  fram.write8(addr, val);
}

int memRead(int addr){
  fram.read8(addr);
}

char* modeText(byte s){
  char* r;
  switch(s) {
    case SMARTMODE:
      r = "Smart";
      break;
    case ECOMODE:
      r = "ECO";
      break;
    case COMFORTMODE:
      r = "Comfort";
      break;
    case SPORTMODE:
      r = "Sport";
      break;
    case CUSTOMMODE:
      r = "Custom";
      break;
     default:
      r = "";
  }
  return r;
}
