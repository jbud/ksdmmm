/**
 * Kia Stinger Drive Mode Memory Module
 * 
 * microcontroller programming in arduino (C++)
 * 
 * by Joe Jackson 2020
 * Version 0.9.3rc
 */
#include <EEPROM.h>

// some arduinos have issues with pulling ground to some pins upon startup.
#define ERROR_OFFSET 1

// Set time to wait before outputing memory to vehicle in ms (recommended 2000-5000)
#define STARTUP_WAIT_TIME 4000

int memAddress = 0; 
int memSetupAddr = 1;
int memISGAddr = 2;

const byte SMARTMODE = 0;
const byte ECOMODE = 1;
const byte COMFORTMODE = 2;
const byte SPORTMODE = 3;
const byte CUSTOMMODE = 4;

byte currentMode;

bool dm_inputAllowed = true;

bool isg = true;
bool isg_inputAllowed = true;


void setup() {
  pinMode(3, INPUT_PULLUP); // clockwise input
  pinMode(4, INPUT_PULLUP); // counterclockwise input
  pinMode(5, OUTPUT);
  pinMode(6, OUTPUT);
  pinMode(7, INPUT_PULLUP); // ISG (Auto Start Stop) input
  pinMode(8, OUTPUT);
  if (!EEPROM.read(memISGAddr)) {
    isg = false; // disable ISG
  }
  //If initial setup hasn't completed, set the default drive mode
  if (EEPROM.read(memSetupAddr) != 1) { 
    // Write the default to permanent memory
    EEPROM.write(memAddress, COMFORTMODE);
    EEPROM.write(memSetupAddr, 1);
    currentMode = COMFORTMODE;
  } else {
    // Setup has completed read the current mode from permanent memory
    currentMode = EEPROM.read(memAddress); 
  }

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
        break;
  }
  // if ISG is disabled in memory, send a signal. ISG is on by default.
  if (!isg){
    digitalWrite(8, HIGH);
    delay(250);
    digitalWrite(8, LOW);
  }
}

void loop() {
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
    EEPROM.write(memAddress, currentMode);
    dm_inputAllowed = false;
  }
  if (digitalRead(3) == LOW && dm_inputAllowed){
    if (currentMode <= SMARTMODE){
      currentMode = SMARTMODE; 
    } else {
      currentMode--;
    }
    EEPROM.write(memAddress, currentMode);
    dm_inputAllowed = false;
  }
  if (digitalRead(7) == HIGH && !isg_inputAllowed){
    isg_inputAllowed = true;
  }
  if (digitalRead(7) == LOW && isg_inputAllowed){
    isg = !isg; // reverse the value from current.
    EEPROM.write(memISGAddr, isg);
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
  }
}

// simulate a counterclockwise turn of drive mode num times
void counterClockWise(int num){
  for(int i=0;i<num;i++){
    digitalWrite(6, HIGH);
    delay(250);
    digitalWrite(6, LOW);
    delay(250);
  }
}
