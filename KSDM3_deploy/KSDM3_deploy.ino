/**
 * Kia Stinger Drive Mode Memory Module
 * 
 * microcontroller programming in arduino (C++)
 * 
 * by Joe Jackson 2020
 * Version 1.0
 */
#include <EEPROM.h>

#define OFFSET 1

#define STARTUP_WAIT_TIME 4000

int mDMAddr = 0;
int mSetupAddr = 1;
int mISGAddr = 2;
int mAHoldAddr = 3;

const byte SMART    = 0; // b00000000
const byte ECO      = 1; // b00000001
const byte COMFORT  = 2; // b00000010
const byte SPORT    = 3; // b00000011
const byte CUSTOM   = 4; // b00000100

byte currentMode;
bool isg;
bool ahold;

bool dm_i = true;
bool ahold_i = true;
bool isg_i = true;

void setup() {
  pinMode(3, INPUT_PULLUP); // clockwise input
  pinMode(4, INPUT_PULLUP); // counterclockwise input
  pinMode(5, OUTPUT);
  pinMode(6, OUTPUT);
  pinMode(7, INPUT_PULLUP); // ISG (Auto Start Stop) input
  pinMode(8, OUTPUT);
  pinMode(9, INPUT); // Auto Hold input
  pinMode(10, OUTPUT);

  if (EEPROM.read(mSetupAddr) != 1) { 
    // Write the default to permanent memory
    EEPROM.write(mDMAddr, COMFORT);
    EEPROM.write(mISGAddr, true);
    EEPROM.write(mAHoldAddr, false);
    EEPROM.write(mSetupAddr, 1);

    currentMode = COMFORT;
    isg = true;
    ahold = false;
  } else {
    // Setup has completed read the current mode from permanent memory
    currentMode = EEPROM.read(mDMAddr);
    if (EEPROM.read(mAHoldAddr)) {
      ahold = true; // enable Autohold
    }
    if (!EEPROM.read(mISGAddr)) {
      isg = false; // disable ISG
    }
  }

  delay(STARTUP_WAIT_TIME);

  switch(currentMode){
    case SPORT:
      clockWise(1 - OFFSET);
      break;
    case CUSTOM:
      clockWise(2 - OFFSET);
      break;
    case SMART:
      counterClockWise(2 + OFFSET);
      break;
    case COMFORT:
      counterClockWise(OFFSET);
      break;
    case ECO:
      counterClockWise(OFFSET);
      break;
  }
  // ****
  // Autohold is triggered on HIGH instead of LOW like the other functions, 
  // this is implemented mechanically and programatically remains like the others.
  // ****

  // if Autohold is enabled in memory, send a signal. Autohold is off by default.
  if (ahold){
    digitalWrite(10, HIGH);
    delay(250);
    digitalWrite(10, LOW);
  }

  // if ISG is disabled in memory, send a signal. ISG is on by default.
  if (!isg){
    digitalWrite(8, HIGH);
    delay(250);
    digitalWrite(8, LOW);
  }
  
}


void loop() {
  if (digitalRead(4) == HIGH && digitalRead(3) == HIGH && !dm_i){
    dm_i = true;
  }

  if (digitalRead(7) == HIGH && !isg_i){
    isg_i = true;
  }

  if (digitalRead(9) == LOW && !ahold_i){
    ahold_i = true;
  }

  if (digitalRead(3) == LOW && dm_i){
    if (currentMode >= CUSTOM){
      currentMode = CUSTOM; 
    } else {
      currentMode++;
    }
    EEPROM.write(mDMAddr, currentMode);
    dm_i = false;
  }

  if (digitalRead(4) == LOW && dm_i){
    if (currentMode <= SMART){
      currentMode = SMART; 
    } else {
      currentMode--;
    }
    EEPROM.write(mDMAddr, currentMode);
    dm_i = false;
  }
  
  if (digitalRead(7) == LOW && isg_i){
    isg = !isg; // reverse the value from current.
    EEPROM.write(mISGAddr, isg);
    isg_i = false;
  }

  if (digitalRead(9) == HIGH && ahold_i){
    ahold = !ahold; // reverse the value from current.
    EEPROM.write(mAHoldAddr, ahold);
    ahold_i = false;
  }
}

// simulate a clockwise turn of drive mode num times
void clockWise(int num){
  for(int i=0;i<num;i++){
    digitalWrite(5, HIGH);
    delay(500);
    digitalWrite(5, LOW);
    delay(500);
  }
}

// simulate a counterclockwise turn of drive mode num times
void counterClockWise(int num){
  for(int i=0;i<num;i++){
    digitalWrite(6, HIGH);
    delay(500);
    digitalWrite(6, LOW);
    delay(500);
  }
}
