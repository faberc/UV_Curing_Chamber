/*****************************************************************************
 * "main.ino"
 *
 * Description:
 *     This file holds the primary device controls that control power overflow
 *     through the curing chamber.
 *
 * - Things that need to be added: DEBUG menu (set variables so that we can test
 * one thing at a time.)
 * - Adjust motor speed by menu
 * - The board stepper works, but the program is pausing for some reason
 * 
 *
 *****************************************************************************/

 ///////////////////////////////////////////////////////////////////////////////
 //--- Temperature Sensor Headers ---//
 ///////////////////////////////////////////////////////////////////////////////

#include <Wire.h>
#include <Stepper.h>
#include "Adafruit_MCP9808.h"

///////////////////////////////////////////////////////////////////////////////
//--- Pin Variables ---//
///////////////////////////////////////////////////////////////////////////////

// Pin Assignments
#define RELAY 4
#define T1 16   // DM Transistor
#define T2 33   // Heating Pad Transistor

  // Stepper Definitions
#define ME 12   // Motor Enable for both
#define M1 26   // Motor Pin 1
#define M2 27   // Motor Pin 2
#define M3 13   // Motor Pin 3
#define M4 14   // Motor Pin 4
#define SPR 200 // Steps per revolution for Stepper
#define MOTOR_SPEED 10 // Speed in revolutions per minute

// #define PWM_CHANNEL 0 // Channel for analog writing to motor // FIXME: May not need
// #define MOTOR_SPEED 50 // PWM of 100/255 on motor enable pin. // FIXME: May not need

// ESP32 Board TFT I2C Interface Pinout
#define TFT_CS        5
#define TFT_SCLK      18
#define TFT_MOSI      23
#define TFT_DC        19
#define TFT_RST       17

// GPIO PINS
#define ROT_A 32
#define ROT_B 35
#define BUTT_PRESS 34

///////////////////////////////////////////////////////////////////////////////
//--- Global Constants ---//
///////////////////////////////////////////////////////////////////////////////

// Number of days between reset to handle millis() overflow
#define RESET_DAYS 25

// Preset Memory Locations
#define P1_TEMP 0
#define P1_TIME 1
#define P2_TEMP 2
#define P2_TIME 3
#define P3_TEMP 4
#define P3_TIME 5
#define P4_TEMP 6
#define P4_TIME 7
#define P5_TEMP 8
#define P5_TIME 9
#define EEPROM_SIZE 10

///////////////////////////////////////////////////////////////////////////////
//--- Global Variables ---//
///////////////////////////////////////////////////////////////////////////////

// Global Variables
bool cureTrigger = 0;  // Has curing been initiated?
bool cureState = 0;   // Is curing in process?
bool dmStatus = 0;    // Dead Man Switch Status
bool dmTrigger = 0;   // DM switch triggered for resume state
bool cureComplete = 0; // Did cure complete.
bool rising = 0;
bool pauseTrigger = 0; // Pauses curing
bool pauseState = 0;
bool resumeTrigger = 0; // resume after pausing
bool cancelTrigger = 0; // Cancels curing

// Cure Control Variables
float             cureTemp = 0;   // in C comes from curing chamber menu setting
unsigned long int cureTime = 0;   // in ms comes from curing chamber menu setting
long int          cureProgress = 0; // in ms This is used to count down the cure time
float             tempLower = 0;
float             tempUpper= 0;
float             currentTemp = 0; // in C current temperature

// Resin Values [temperature in C, time in minutes]
int regResin[2] = {60, 30};
int colorResin[2] = {60, 30};
int gpResin[2] = {80, 15};
int rigidResin [2] = {80, 15};
int toughResin [2] = {60, 60};
int durResin [2] = {60, 60};
int highResin [2] = {60, 60};
int castResin [2] = {45, 280};
int preset1Resin[2] = {};
int preset2Resin[2] = {};
int preset3Resin[2] = {};
int preset4Resin[2] = {};
int preset5Resin[2] = {};

///////////////////////////////////////////////////////////////////////////////
//--- Curing Chamber Menu Interface Header ---//
///////////////////////////////////////////////////////////////////////////////

// Include Menu Code
#include "Curing_Chamber_Variables.h"

///////////////////////////////////////////////////////////////////////////////
//--- Device Setup Functions ---//
///////////////////////////////////////////////////////////////////////////////

// Create the MCP9808 temperature sensor object
Adafruit_MCP9808 tempsensor = Adafruit_MCP9808();

// Create Instance of Stepper LIbrary
Stepper myStepper(SPR, M1, M2, M3, M4);

void setup() {

  // Start Serial Monitor
  Serial.begin(9600);

  // Initialize the Temperature Sensor
  if (!tempsensor.begin(0x18))
    {
    Serial.println("Couldn't find MCP9808!");
    while (1);
    }

  // Initalize EEPROM and preset values
  if (!EEPROM.begin(EEPROM_SIZE)) {
    Serial.println("failed to initialise EEPROM"); delay(1000000);
  }
  
  preset1Resin[0] = EEPROM.read(P1_TEMP);
  preset1Resin[1] = EEPROM.read(P1_TIME);
  preset2Resin[0] = EEPROM.read(P2_TEMP);
  preset2Resin[1] = EEPROM.read(P2_TIME);
  preset3Resin[0] = EEPROM.read(P3_TEMP);
  preset3Resin[1] = EEPROM.read(P3_TIME);
  preset4Resin[0] = EEPROM.read(P4_TEMP);
  preset4Resin[1] = EEPROM.read(P4_TIME);
  preset5Resin[0] = EEPROM.read(P5_TEMP);
  preset5Resin[1] = EEPROM.read(P5_TIME);

  // Setup I/O
  pinMode(RELAY, OUTPUT);
  pinMode(T1, INPUT); // Reads status of DM switch
  pinMode(T2, OUTPUT); // Controls triggering of heating pad

  // Setup Stepper Motor Speed
  myStepper.setSpeed(MOTOR_SPEED);

//pinMode(ME, OUTPUT);
//  ledcSetup(PWM_CHANNEL, 5000, 8); // Create a PWM channel '0' with freq 5000Hz, and resolution of 8
//  ledcAttachPin(ME, PWM_CHANNEL);  // Attach motor enable to PWM channel.


  digitalWrite(RELAY, 0);   // Initialize RELAY to off mode

  // Rotary Encoder Setup
  pinMode(ROT_A, INPUT);
  pinMode(ROT_B, INPUT);
  pinMode(BUTT_PRESS, INPUT);

  // Initialize LED Screen
  tft.initR(INITR_BLACKTAB);
  tft.setRotation(3);

  startup_screen();
  intialize_menu();
  display_menu(menu, option, 0);

}

///////////////////////////////////////////////////////////////////////////////
//--- Mainline Loop ---//
///////////////////////////////////////////////////////////////////////////////

void loop() {
  // Reads temperature in from temp sensor
  getTemp();


  // Updates the display based on menu structures and inputs
  update_display();

  // Initializes the curing process when the cure trigger is set
  if (cureTrigger)
    initializeCure();

// Controls the cureing process based on a set procedure
  if (cureState) 
    cureProcedure();

  // Overflow Error Avoidance: Reset ESP32 once every 25 days
  if ((millis() / (RESET_DAYS * 24 * 60 * 60 * 1000)) >= 1 )
    ESP.restart();
}

/*****************************************************************************
 * getTemp
 *
 * Description:
 *     Reads the temperature sensor temp value into the currentTemp variable.
 *
 * @params - none.
 * @returns - none.
 *****************************************************************************/

void getTemp() {
  // Reads the temperature into currentTemp for the
  currentTemp = tempsensor.readTempC();

  // Debugging block
  if ( (millis()%5000) < 1) {
    Serial.print("\nTemp: ");
    Serial.print(currentTemp);
    Serial.print(" C");
  }
}

void initializeCure () {
    Serial.println("\nCuring has started");
    cureTime = cureTime * 1000 * 60;
    cureProgress = cureTime;
    cureTime += millis();
    tempLower = cureTemp - 5;
    tempUpper = cureTemp + 5;
    cureTrigger = 0;
    rising = 0;   // FIXME: Test to see if value 0 works
    cureState = 1;
    digitalWrite(RELAY, 1);

    display_menu(6, cureProgress/60000, currentTemp);  // FIXME: Chuck Added to test immediately updating the progress

    
//    digitalWrite(M1, 1);  FIXME: REPLACE WITH STEPPER FUNCTION
//    digitalWrite(M2, 0);
//    ledcWrite(PWM_CHANNEL, MOTOR_SPEED);

    delay(10);
}


void cureProcedure() {
  // Handles the whole curing process itself

  // End of cure response
  if (cureProgress <= 0) {   // If the remaining cure time is less than 0
    Serial.println("\nCuring has completed.");
    cureComplete = 1;
    digitalWrite(RELAY, 0);
//    ledcWrite(PWM_CHANNEL, 0);
//    digitalWrite(ME, 0);  // Write the Motor Enable pin low -- no longer needed
    cureState = 0;
  }

  // Curing in-progress protocol
  else {

    // Debugging block for remaining time
    if (millis() % 10000 < 1) {
      Serial.print("\nRemaining Time: ");
      Serial.print(cureProgress/60000);
      Serial.print("\n");
    }

    // Initializes a pause event
    if (pauseTrigger) {
      pauseTrigger = 0;
      pauseState = 1;
      Serial.println("\nPause State Triggered");
      digitalWrite(RELAY, 0);
      Serial.println(digitalRead(RELAY));
    }

    // When paused, continuously increments the cure time
    if (pauseState) {
      cureTime = cureProgress + millis();
    } else {
      cureProgress = cureTime - millis();   // If door is closed decrement cureProgress by how many millis have passed.
      // Runs the stepper 5 steps at a time
      myStepper.step(1);  
    }

    // Initializes a resume event
    if (resumeTrigger) {
      Serial.println("\nResume State Triggered");
      resumeTrigger = 0;
      pauseState = 0;
      digitalWrite(RELAY, 1);
      Serial.println(digitalRead(RELAY));
    }

    // Initializes a cancel event
    if (cancelTrigger) {
      cureProgress = 0;
      pauseState = 0;
      Serial.println("\nJob has been Canceled.");

      // Print Remaining time to check that it is 0.
      Serial.print("\nRemaining Time: ");
      Serial.print(cureProgress/60000);
      Serial.print("\n");

      cancelTrigger = 0;
    }

////////////////////////////////////////////////////////
// Deadman's Switch Code: Code to run if door is open //
////////////////////////////////////////////////////////

    // Reads the deadman's switch
    dmStatus = digitalRead(T1);

    // Code to handle dead-man switch
    // If the door is open, keep adding the current clock time to the time remaining to save the time.
    if (dmStatus == 1) {
      // Debugging serial print
      if ((millis() % 10000) < 1) {
        Serial.println("\nDoor is open. Close door to continue curing.");
      }
      pauseTrigger = 1; // Pauses job when door is open. FIXME (uncomment when done testing)********
      dmTrigger = 1;
    }

    // If coming out of a pause state and door is closed
    else {
      if (dmTrigger && pauseState) {
        dmTrigger = 0;                      // Turn off the trigger
        resumeTrigger = 1;                  // resumes the print.
      }
 ////////////////////////////////////////////////////////////////////////////////
 // Code which runs while the door is closed and the curing process is working.//
 ////////////////////////////////////////////////////////////////////////////////
      
//      cureProgress = cureTime - millis();   // If door is closed decrement cureProgress by how many millis have passed.
//      // Runs the stepper 5 steps at a time
//      myStepper.step(1);      
    }

////////////////////////////
// Temperature Regulation //
////////////////////////////
    
    // Turns OFF heater pad
    if (currentTemp > tempUpper && rising) {
      // Debugging block
      Serial.println("\nChamber is now cooling.");
      rising = 0;
      digitalWrite(T2, 0);
    }

    // Turns ON the heater pad
    else if (currentTemp < tempLower && !rising) {
        Serial.println("\nChamber is now heating.");
        rising = 1;
        digitalWrite(T2, 1);
      }
    }
}
