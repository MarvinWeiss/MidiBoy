/*
   Name: MidiBoy
   Description: This Script powers a small midi device, which is meant to controll knobs and buttons in any DAW
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



#define BUTTON_START_PIN 2

#define BUTTON_1_PIN 2
#define BUTTON_2_PIN 3
#define BUTTON_3_PIN 4
#define BUTTON_4_PIN 5

//If the LED Pins are in cronological order use Start Pin and a loop to declare Button LEDs
#define BUTTON_LED_START_PIN 6

#define BUTTON_LED_1_PIN 6
#define BUTTON_LED_2_PIN 7
#define BUTTON_LED_3_PIN 8
#define BUTTON_LED_4_PIN 9


#define  POTI_START_PIN A0
#define POTI_1_PIN A0
#define POTI_2_PIN A1
#define POTI_3_PIN A2
#define POTI_4_PIN A3


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



#define BUTTON_SETTING


#define MIDI_BUTTON_HIGH 127
#define MIDI_BUTTON_LOW 0


//The sum of all Buttons, Potis or else
#define MIDI_COMPONENTS 8

#define BUTTON_AMOUNT 4
#define POTI_AMOUNT 4

#define MIDI_CHANNEL 2



const int SMOOTH_FACTOR = 5;


const int POTI_SKIP_FACTOR = 10;


Bounce button1 = Bounce();
Bounce button2 = Bounce();
Bounce button3 = Bounce();
Bounce button4 = Bounce();


int currentValuePoti1 = -1;
int currentValuePoti2 = -1;
int currentValuePoti3 = -1;
int currentValuePoti4 = -1;


int potiValue1;
int potiValue2;
int potiValue3;
int potiValue4;

bool button1State = false;
bool button2State = false;
bool button3State = false;
bool button4State = false;


bool button1Pressed;
bool button2Pressed;
bool button3Pressed;
bool button4Pressed;

int potiSkipCounter = 0;

bool sendMidiMessages;

bool sendButtonMidiMessages;

bool sendPotiMidiMessages;


midiEventPacket_t midiMessages[MIDI_COMPONENTS] = {};

midiEventPacket_t buttonMidiMessages[BUTTON_AMOUNT] = {};
midiEventPacket_t potiMidiMessages[POTI_AMOUNT] = {};

int midiMessageCounter = 0;

int buttonMidiMessageCounter = 0;
int potiMidiMessageCounter = 0;


Button buttons [BUTTON_AMOUNT];

Poti potis [POTI_AMOUNT];



void setup() {

  //Serial.begin(9600);
  //while (!Serial);


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



  for (int i = 0; i < POTI_AMOUNT ; i++) {

    potis[i].potiPin = POTI_START_PIN + i;
    potis[i].potiValue = -1;
    potis[i].newPotiValue = -1;
    potis[i].midiControl = POTI_CONTROL_STARTADDRESS + i;

  }

  LEDsStartuppatern();

  //
  //  button1.attach(BUTTON_1_PIN, INPUT);
  //  button2.attach(BUTTON_2_PIN, INPUT);
  //  button3.attach(BUTTON_3_PIN, INPUT);
  //  button4.attach(BUTTON_4_PIN, INPUT);
  //
  //  button1.interval(10);
  //  button2.interval(10);
  //  button3.interval(10);
  //  button4.interval(10);

  ////Serial.println("Ready!");
  wdt_enable( WDTO_2S);
}




void loop() {

  wdt_reset();


  //if(notifyCounter % 5){
  //Serial.println("Running!");
  //}
  //notifyCounter++;

  //  button1.update();
  //  button2.update();
  //  button3.update();
  //  button4.update();
  //
  //
  //  button1Pressed = button1.rose();
  //  button2Pressed = button2.rose();
  //  button3Pressed = button3.rose();
  //  button4Pressed = button4.rose();
  //

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
      //MidiUSB.sendMIDI(controlChange(MIDI_CHANNEL, buttons[i].midiControl, value));
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
      //potis[i].newPotiValue = (potis[i].potiValue - (potis[i].potiValue  >> SMOOTH_FACTOR)) + (potis[i].newPotiValue >> SMOOTH_FACTOR);
      
      
      //Serial.print("\t");
     //potis[i].newPotiValue = (6*potis[i].potiValue + 4*potis[i].newPotiValue);//map((6*potis[i].potiValue + 4*potis[i].newPotiValue)/10, 0, 1023, 0, 127);
      //Serial.println( potis[i].newPotiValue);
      if (potis[i].potiValue  != potis[i].newPotiValue ) {
        potis[i].potiValue = potis[i].newPotiValue;
        //MidiUSB.sendMIDI(controlChange(MIDI_CHANNEL, potis[i].midiControl, potis[i].potiValue));

        potiMidiMessages[ potiMidiMessageCounter ] = controlChange(MIDI_CHANNEL, potis[i].midiControl, potis[i].potiValue);
        sendPotiMidiMessages = true;
        potiMidiMessageCounter++;

      }
//      else if ( potis[i].potiValue == -1) {
//        potis[i].potiValue = potis[i].newPotiValue;
//      }

    }
  } else {
    potiSkipCounter++;
  }

  if (sendButtonMidiMessages) {

    sendMidiPackages(buttonMidiMessages, buttonMidiMessageCounter);
    ////Serial.println("Buttons Sent");

  }

  if (sendPotiMidiMessages) {

    sendMidiPackages(potiMidiMessages, potiMidiMessageCounter);
    ////Serial.println("Potis Sent");

  }

  MidiUSB.flush();
  delay(10);

}





void sendMidiPackages ( midiEventPacket_t messages [], int midiMessageAmount) {
  //if (malloc(sizeof(messages)) != 0) {
  for (int i = 0; i < midiMessageAmount ; i++) {
    MidiUSB.sendMIDI(messages[i]);
  }

  //}
}


// First parameter is the event type (0x09 = note on, 0x08 = note off).
// Second parameter is note-on/note-off, combined with the channel.
// Channel can be anything between 0-15. Typically reported to the user as 1-16.
// Third parameter is the note number (48 = middle C).
// Fourth parameter is the velocity (64 = normal, 127 = fastest).

//void noteOn(byte channel, byte pitch, byte velocity) {
//  midiEventPacket_t noteOn = {0x09, 0x90 | channel, pitch, velocity};
//  MidiUSB.sendMIDI(noteOn);
//}
//
//void noteOff(byte channel, byte pitch, byte velocity) {
//  midiEventPacket_t noteOff = {0x08, 0x80 | channel, pitch, velocity};
//  MidiUSB.sendMIDI(noteOff);
//}


// First parameter is the event type (0x0B = control change).
// Second parameter is the event type, combined with the channel.
// Third parameter is the control number number (0-119).
// Fourth parameter is the control value (0-127).
//
// Further information about MIDI Controll changes:
// https://www.midi.org/specifications-old/item/table-3-control-change-messages-data-bytes-2


midiEventPacket_t controlChange(byte channel, byte control, byte value) {
  midiEventPacket_t event = {0x0B, 0xB0 | channel, control, value};
  //MidiUSB.sendMIDI(event);
  return event;
}


void clearMidiPacketArray() {
  if (malloc(sizeof(midiMessages)) != 0) {
    for (int i = 0; malloc(sizeof(midiMessages[i])) != 0 && i < MIDI_COMPONENTS ; i++) {
      //delete midiArray[i];
      midiMessages[i] = {};
    }
  }
}




void updateButtons() {
  for (int i = 0 ; i < BUTTON_AMOUNT; i++) {
    buttons[i].bounce.update();
    buttons[i].buttonPressed = buttons[i].bounce.rose();
  }
}









//Turns all LEDs on or off

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
