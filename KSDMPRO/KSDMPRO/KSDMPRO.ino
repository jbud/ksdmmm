/**
 * Kia Stinger Drive Mode Memory Module PRO
 * 
 * microcontroller programming in arduino (C++)
 * 
 * Uses ATMEGA 2560 w/ Arduino Bootloader
 * 
 * by Joe Jackson 2020
 * Version 0.9
 */
#include <Wire.h>
#include <FRAM.h>

//#define EEPROMMODE
//#include <EEPROM.h>

// Set time to wait before outputing memory to vehicle in ms (recommended 2000-5000)
#define STARTUP_WAIT_TIME 4000
// Set whether to use automatic electronic parking brake (0 off, 1 on)
#define USE_AUTO_PARKING_BRAKE 1
// Set whether to disable Traction and Stability control for CUSTOM (0 off, 1 on)
#define USE_RACE_MODE 1
// Set if two temp seats or one (0 = Driver side hot and cold, 1 = driver hot and passenger hot)
#define TWO_TEMP_SEATS 1
// May be needed.
#define ERROR_OFFSET 0

// ## Define pins
//Inputs
#define DMRI  4  // Drive Mode Right
#define DMLI  0  // Drive Mode Left
#define ISGI  1  // ISG
#define AHI   5  // Autohold
#define TRCI  41 // Traction Control
#define HSI   40 // Heated Seat (driver)
#define VSI   37 // Ventalated Seat (driver) / Heated Seat (passenger)
#define HSWI  36 // Heated Steering Wheel
#define PRKI  42 // PARK signal (LED from park trigger)
#define FUTI  19 // Future1 Input

//Outputs
#define DMRO  2  // Drive Mode Right
#define DMLO  3  // Drive Mode Left
#define ISGO  17 // ISG
#define AHO   16 // Autohold
#define TRCO  35 // Traction Control
#define HSO   34 // Heated Seat (driver)
#define VSO   33 // Ventalated Seat (driver) / Heated Seat (passenger)
#define HSWO  32 // Heated Steering Wheel
#define EPBO  18 // Electric Parking Brake
#define FUTO  38 // Future1 Output 
#define LEPO  6  // LED Option 0 Indicator
#define LTRO  7  // LED Option 1 Indicator


// ## Define byte settings
#define SMART 0
#define ECO 1
#define COMFORT 2
#define SPORT 3
#define CUSTOM 4

#define TRACON 0   // Traction Control Enabled
#define TRACOFF 1  // Traction Control Disabled
#define ESCOFF 2   // Traction and Stability Control Disabled

// generic seat settings 3 levels and off
#define SEAT0 0
#define SEAT1 1
#define SEAT2 2
#define SEAT3 3

// ## Define Memory Addresses
// Storage
const int mDriveMode = 0x01;
const int mISG = 0x02;
const int mAutohold = 0x03;
const int mTractionMode = 0x04;
const int mHeatedSeat = 0x05;
const int mCooledSeat = 0x06;
const int mHeatedWheel = 0x07;
// Preferences
const int mpAutoEPBEnable = 0x08; // Enable Electronic Parking Brake on shift to P
const int mpReplaceCustom = 0x09; // Replace Custom drive mode with TRACK mode (Automatically disable Traction and ESC)
const int mSetup = 0x0A;

byte currentMode;
bool isg;
bool ahold;
byte traction;
byte seatHeat;
byte seatCool;
bool wheelHeat;

bool aepb;
bool raceMode;

// ## polling switches
//Standard
bool dm_i = true;
bool isg_i = true;
bool ahold_i = true;
bool traction_i = true;
bool seatHeat_i = true;
bool seatCool_i = true;
bool wheelHeat_i = true;
//Special
bool pInd_i = true;
bool tChange;

unsigned long t;
unsigned long p;

#ifndef EEPROMMODE
FRAM fram;
#endif
void setup() {
 #ifndef EEPROMMODE 
    Wire.begin();
    fram.begin(0x50);
 #endif
    // ## set pin modes
    //inputs
    pinMode(DMRI, INPUT_PULLUP);
    pinMode(DMLI, INPUT_PULLUP);
    pinMode(ISGI, INPUT_PULLUP);
    pinMode(AHI, INPUT_PULLUP);
    pinMode(TRCI, INPUT_PULLUP);
    pinMode(HSI, INPUT_PULLUP);
    pinMode(VSI, INPUT_PULLUP);
    pinMode(HSWI, INPUT_PULLUP);
    pinMode(PRKI, INPUT_PULLUP);
    pinMode(FUTI, INPUT_PULLUP);
    //outputs
    pinMode(DMRO, OUTPUT);
    pinMode(DMLO, OUTPUT);
    pinMode(ISGO, OUTPUT);
    pinMode(AHO, OUTPUT);
    pinMode(TRCO, OUTPUT);
    pinMode(HSO, OUTPUT);
    pinMode(VSO, OUTPUT);
    pinMode(HSWO, OUTPUT);
    pinMode(EPBO, OUTPUT);
    pinMode(FUTO, OUTPUT);
    pinMode(LEPO, OUTPUT);
    pinMode(LTRO, OUTPUT);
    
    
    // ## initial Setup (defaults)
    if (memRead(mSetup) != 0x1F){
      currentMode = COMFORT;
      memWrite(mDriveMode, currentMode);
      isg = true;
      memWrite(mISG, isg);
      ahold = false;
      memWrite(mAutohold, ahold);
      traction = TRACON;
      memWrite(mTractionMode, traction);
      seatHeat = SEAT0;
      memWrite(mHeatedSeat, seatHeat);
      seatCool = SEAT0;
      memWrite(mCooledSeat, seatCool);
      wheelHeat = false;
      memWrite(mHeatedWheel, wheelHeat);
      aepb = USE_AUTO_PARKING_BRAKE;
      memWrite(mpAutoEPBEnable, aepb);
      raceMode = USE_RACE_MODE;
      memWrite(mpReplaceCustom, raceMode);
      
      // Setup complete, set flag.
      memWrite(mSetup, 0x1F);
    }
    
    // ## read settings from memory
    currentMode = memRead(mDriveMode);
    isg = memRead(mISG);
    ahold = memRead(mAutohold);
    traction = memRead(mTractionMode);
    seatHeat = memRead(mHeatedSeat);
    seatCool = memRead(mCooledSeat);
    wheelHeat = memRead(mHeatedWheel);
    aepb = memRead(mpAutoEPBEnable);
    raceMode = memRead(mpReplaceCustom);

    
    delay(STARTUP_WAIT_TIME);

    
    if (aepb){
      digitalWrite(LEPO, HIGH);
    }
    if (raceMode){
      digitalWrite(LTRO, HIGH);
    }
    if (ahold){
      digitalWrite(AHO, HIGH);
      delay(250);
      digitalWrite(AHO, LOW);
    }
    // if ISG is disabled in memory, send a signal. ISG is on by default.
    if (!isg){
      digitalWrite(ISGO, HIGH);
      delay(250);
      digitalWrite(ISGO, LOW);
    }
    if (wheelHeat){
      digitalWrite(HSWO, HIGH);
      delay(250);
      digitalWrite(HSWO, LOW);
    }
    if (seatHeat > SEAT0 && ((seatCool == SEAT0) || TWO_TEMP_SEATS)) { // seatHeat=1, seatcool=2, TWO_TEMP_SEATS=1 
      seatTempAdj(seatHeat, false);
    }
    if (seatCool > SEAT0 && ((seatHeat == SEAT0) || TWO_TEMP_SEATS)) {
      seatTempAdj(seatCool, true);
    }
    switch(currentMode){
      case SPORT:
        clockWise(1-ERROR_OFFSET);
        break;
      case CUSTOM:
        clockWise(2-ERROR_OFFSET);
        break;
      case SMART:
        counterClockWise(2+ERROR_OFFSET);
        break;
      case COMFORT:
        counterClockWise(ERROR_OFFSET);
        break;
      case ECO:
        counterClockWise(ERROR_OFFSET);
        break;
      default:
        break;
    }
    if (traction == TRACOFF){
      digitalWrite(TRCO, HIGH);
      delay(250);
      digitalWrite(TRCO, LOW);
    }
    if (traction == ESCOFF){
      disableESC();
    }

}

void loop() {
  bool tCustom = false;
  if (digitalRead(DMRI) == HIGH && digitalRead(DMLI) == HIGH && !dm_i){
    dm_i = true;
  }
  if (digitalRead(ISGI) == HIGH && !isg_i){
    isg_i = true;
  }
  if (digitalRead(AHI) == HIGH && !ahold_i){
    ahold_i = true;
  }
  if (digitalRead(TRCI) == HIGH && !traction_i){
    traction_i = true;
  }
  if (digitalRead(HSI) == HIGH && !seatHeat_i && ((digitalRead(VSI) == HIGH) || TWO_TEMP_SEATS)){
    seatHeat_i = true;
  }
  if (digitalRead(VSI) == HIGH && !seatCool_i && ((digitalRead(HSI) == HIGH) || TWO_TEMP_SEATS)){
    seatCool_i = true;
  }
  if (digitalRead(HSWI) == HIGH && !wheelHeat_i){
    wheelHeat_i = true;
  }
  if (digitalRead(PRKI) == LOW && !pInd_i){
    pInd_i = true;
  }
  if (digitalRead(DMRI) == LOW && dm_i){
    if (currentMode >= CUSTOM){
      currentMode = CUSTOM; 
    } else {
      currentMode++;
    }
    if (currentMode == CUSTOM){
      tCustom = true;
    }
    memWrite(mDriveMode, currentMode);
    dm_i = false;
  }
  if (digitalRead(DMLI) == LOW && dm_i){
    if (currentMode <= SMART){
      currentMode = SMART; 
    } else {
      currentMode--;
    }
    memWrite(mDriveMode, currentMode);
    dm_i = false;
  }
  if (digitalRead(ISGI) == LOW && isg_i){
    isg = !isg; // reverse the value from current.
    memWrite(mISG, isg);
    isg_i = false;
  }
  if (digitalRead(AHI) == LOW && ahold_i){
    ahold = !ahold; // reverse the value from current.
    memWrite(mAutohold, ahold);
    ahold_i = false;
  }
  if (digitalRead(HSWI) == LOW && wheelHeat_i){
    wheelHeat = !wheelHeat; // reverse the value from current.
    memWrite(mISG, isg);
    wheelHeat_i = false;
  }
  if (digitalRead(HSI) == LOW && seatHeat_i){
    seatHeat++;
    if (seatHeat > SEAT3){
      seatHeat = SEAT0; // Loop Back
    }
    if (!TWO_TEMP_SEATS){
      seatCool = SEAT0;
      memWrite(mCooledSeat, seatCool);
    }
    memWrite(mHeatedSeat, seatHeat);
    seatHeat_i = false;
  }
  if (digitalRead(VSI) == LOW && seatCool_i){
    seatCool++;
    if (seatCool > SEAT3){
      seatCool = SEAT0; // Loop Back
    }
    if (!TWO_TEMP_SEATS){
      seatHeat = SEAT0;
      memWrite(mHeatedSeat, seatHeat);
    }
    memWrite(mCooledSeat, seatCool);
    seatCool_i = false;
  }

  if (digitalRead(TRCI) == LOW){
    if (traction_i){
      p = millis();
      traction_i = false;
    }
    t = millis();
    if (((t - p) > 4500) && !traction_i){ // trigger at 4500 to be safe.
      traction = ESCOFF;
      tChange = true;
    } else {
      if (!traction_i){
        if (traction == TRACON){
          traction = TRACOFF;
        }
        if (traction == TRACOFF || traction == ESCOFF){
          traction = TRACON;
        }
        tChange = true;
      }
    }
    if (tChange){
      memWrite(mTractionMode, traction);
    }
  }
  
  if (digitalRead(PRKI) == HIGH && pInd_i && aepb){
    digitalWrite(EPBO, HIGH);
    delay(500); // 0.5 seconds
    digitalWrite(EPBO, LOW);
  }
  if (tCustom && raceMode) {
    disableESC();
    tCustom = false;
  }
}

void disableESC(){
  digitalWrite(TRCO, HIGH);
  delay(5250); // 5.25 seconds
  digitalWrite(TRCO, LOW);
}

void seatTempAdj(int amt, bool cool){
  int p = cool ? VSO : HSO;
  for(int i=0;i<amt;i++){
    digitalWrite(p, HIGH);
    delay(250);
    digitalWrite(p, LOW);
    delay(250);
  }
}

// simulate a clockwise turn of drive mode num times
void clockWise(int num){
  for(int i=0;i<num;i++){
    digitalWrite(DMRO, HIGH);
    delay(250);
    digitalWrite(DMRO, LOW);
    delay(250);
  }
}

// simulate a counterclockwise turn of drive mode num times
void counterClockWise(int num){
  for(int i=0;i<num;i++){
    digitalWrite(DMLO, HIGH);
    delay(250);
    digitalWrite(DMLO, LOW);
    delay(250);
  }
}

void memWrite(int addr, int val){
#ifdef EEPROMMODE
  EEPROM.write(addr, val);
#else
  fram.write8(addr, val);
#endif
}

int memRead(int addr){
#ifdef EEPROMMODE
  return EEPROM.read(addr);
#else
  return fram.read8(addr);
#endif
}
