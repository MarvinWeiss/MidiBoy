/*
   Name: MidiBoy
   Description: This code is meant for a small Midi Device with Potentiometsr, Buttons and LEDs to controll any daw
   Author: Marvin Weiß
*/

//Library which debounces button signals
#include <Bounce2.h>
//Send signals as native MIDI device
#include "MIDIUSB.h"
//Watchdog library
#include <avr/wdt.h>
#include "Button.h"
#include "Potentiometer.h"



//IF the Pins of any in or outputs ar in Order the Start_Pin can be used to automatically initialize them
//Else change the code to fit to your needs
#define BUTTON_START_PIN 2

#define BUTTON_1_PIN 2
#define BUTTON_2_PIN 3
#define BUTTON_3_PIN 4
#define BUTTON_4_PIN 5

#define BUTTON_LED_START_PIN 6

#define BUTTON_LED_1_PIN 6
#define BUTTON_LED_2_PIN 7
#define BUTTON_LED_3_PIN 8
#define BUTTON_LED_4_PIN 9


#define POTI_START_PIN A0
#define POTI_1_PIN A0
#define POTI_2_PIN A1
#define POTI_3_PIN A2
#define POTI_4_PIN A3


//Change controls to Adresses you need, current adresses are undefined according to https://www.midi.org/specifications-old/item/table-3-control-change-messages-data-bytes-2
#define BUTTON_CONTROL_STARTADDRESS 0x14
#define BUTTON_1_CONTROL 0x14
#define BUTTON_2_CONTROL 0x15
#define BUTTON_3_CONTROL 0x16
#define BUTTON_4_CONTROL 0x17

#define POTI_CONTROL_STARTADDRESS 0x18
#define POTI_1_CONTROL 0x18
#define POTI_2_CONTROL 0x19
#define POTI_3_CONTROL 0x1A
#define POTI_4_CONTROL 0x1B


#define MIDI_BUTTON_HIGH 127
#define MIDI_BUTTON_LOW 0


//The sum of Components
#define MIDI_COMPONENTS 8

#define BUTTON_AMOUNT 4
#define POTI_AMOUNT 4

#define MIDI_CHANNEL 2


//To avoid the potis spam the Midi Channel the poti signal is ignored the amount of times this variable indicates
const int POTI_SKIP_FACTOR = 10;

//Counts how many times the Poti signal was ignored, is reset after POTI_SKIP_FACTOR times
int potiSkipCounter = 0;

//Boolean variables to check if a Midi Messages has to be sent
bool sendMidiMessages;

bool sendButtonMidiMessages;

bool sendPotiMidiMessages;


//Midi Event arrays for different inputs

midiEventPacket_t midiMessages[MIDI_COMPONENTS] = {};

midiEventPacket_t buttonMidiMessages[BUTTON_AMOUNT] = {};
midiEventPacket_t potiMidiMessages[POTI_AMOUNT] = {};


//Counters for different Midimessages

int midiMessageCounter = 0;

int buttonMidiMessageCounter = 0;
int potiMidiMessageCounter = 0;



//Struct Arrays for Buttons and Potentiometers
Button buttons [BUTTON_AMOUNT];

Poti potis [POTI_AMOUNT];



void setup() {

  //Serial.begin(9600);
  //while (!Serial);


  //Button init
  for (int i = 0; i < BUTTON_AMOUNT ; i++) {

    buttons[i].buttonPin = BUTTON_START_PIN + i;
    buttons[i].bounce = Bounce();
    buttons[i].buttonLED = BUTTON_LED_START_PIN + i;
    buttons[i].buttonState = false;
    buttons[i].midiControl = BUTTON_CONTROL_STARTADDRESS + i;

    buttons[i].bounce.attach(buttons[i].buttonPin, INPUT);
    buttons[i].bounce.interval(10);


    pinMode(buttons[i].buttonLED, OUTPUT);
  }


  //Potentiometer init
  for (int i = 0; i < POTI_AMOUNT ; i++) {

    potis[i].potiPin = POTI_START_PIN + i;
    potis[i].potiValue = -1;
    potis[i].newPotiValue = -1;
    potis[i].midiControl = POTI_CONTROL_STARTADDRESS + i;

  }

  LEDsStartuppatern();


  wdt_enable( WDTO_2S);
}


//routine:
// Update button interactions
// Reset all Midi collector variables
// Run Button routine and set Midi Message Array for Buttons
// Same goes for Potentiometers
// Send Midi Messages
// Flush MidiUSB Buffer


void loop() {

  wdt_reset();

  updateButtons();


  sendMidiMessages = false;

  sendButtonMidiMessages = false;
  sendPotiMidiMessages = false;


  midiMessageCounter = 0;

  buttonMidiMessageCounter = 0;
  potiMidiMessageCounter = 0;

  //clearMidiPacketArray();

  int value = -1;
  for (int i = 0 ; i < BUTTON_AMOUNT ; i++) {

    if (buttons[i].buttonPressed) {
      if (buttons[i].buttonState) {
        buttons[i].buttonState = false;
        value = MIDI_BUTTON_LOW;
      } else {
        buttons[i].buttonState = true;
        value = MIDI_BUTTON_HIGH;
      }
      buttonMidiMessages[ buttonMidiMessageCounter ] = controlChange(MIDI_CHANNEL, buttons[i].midiControl, value);
      sendButtonMidiMessages = true;
      buttonMidiMessageCounter++;

      digitalWrite(buttons[i].buttonLED, buttons[i].buttonState);
      buttons[i].buttonPressed = false;
    }
  }

  if (potiSkipCounter == POTI_SKIP_FACTOR) {
    potiSkipCounter = 0;
    for (int i = 0; i < POTI_AMOUNT ; i++) {
      potis[i].newPotiValue = map(analogRead(potis[i].potiPin), 0, 1023, 0, 127);


      if (potis[i].potiValue  != potis[i].newPotiValue ) {
        potis[i].potiValue = potis[i].newPotiValue;
        potiMidiMessages[ potiMidiMessageCounter ] = controlChange(MIDI_CHANNEL, potis[i].midiControl, potis[i].potiValue);
        sendPotiMidiMessages = true;
        potiMidiMessageCounter++;

      }

    }
  } else {
    potiSkipCounter++;
  }

  if (sendButtonMidiMessages) {

    sendMidiPackages(buttonMidiMessages, buttonMidiMessageCounter);
  }

  if (sendPotiMidiMessages) {

    sendMidiPackages(potiMidiMessages, potiMidiMessageCounter);
  }

  MidiUSB.flush();
  delay(10);

}





void sendMidiPackages ( midiEventPacket_t messages [], int midiMessageAmount) {
  for (int i = 0; i < midiMessageAmount ; i++) {
    MidiUSB.sendMIDI(messages[i]);
  }
}






// Returns a Midi event to be used within the MidiUSB Functions
// First parameter is the event type (0x0B = control change).
// Second parameter is the event type, combined with the channel.
// Third parameter is the control number number (0-119).
// Fourth parameter is the control value (0-127).
//
// Further information about MIDI Controll changes:
// https://www.midi.org/specifications-old/item/table-3-control-change-messages-data-bytes-2


midiEventPacket_t controlChange(byte channel, byte control, byte value) {
  midiEventPacket_t event = {0x0B, 0xB0 | channel, control, value};
  return event;
}




// Updates buttons array with the bounce routine
void updateButtons() {
  for (int i = 0 ; i < BUTTON_AMOUNT; i++) {
    buttons[i].bounce.update();
    buttons[i].buttonPressed = buttons[i].bounce.rose();
  }
}









//Functions to interact with all LEDs

void LEDsOn() {

  for (int i = 0 ; i < BUTTON_AMOUNT; i++) {
    digitalWrite(buttons[i].buttonLED, HIGH);
  }

}

void LEDsOff() {

  for (int i = 0 ; i < BUTTON_AMOUNT; i++) {
    digitalWrite(buttons[i].buttonLED, LOW);
  }

}


void LEDsOnDelay(int delayTime) {

  for (int i = 0 ; i < BUTTON_AMOUNT; i++) {
    digitalWrite(buttons[i].buttonLED, HIGH);
    delay(delayTime);
  }

}

void LEDsOffDelay(int delayTime) {

  for (int i = 0 ; i < BUTTON_AMOUNT; i++) {
    digitalWrite(buttons[i].buttonLED, LOW);
    delay(delayTime);
  }

}

void LEDsStartuppatern() {

  LEDsOnDelay(300);

  LEDsOff();


  delay(250);

  LEDsOn();

  delay(250);

  LEDsOff();


  delay(250);

  LEDsOn();

  delay(250);

  LEDsOffDelay(300);

}
