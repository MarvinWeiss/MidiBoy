/*
 * Name: MidiBoy
 * Description: This Script powers a small midi device, which is meant to controll knobs and buttons in any DAW
 * Author: Marvin Weiß
 */

//Library which debounces button signals
#include <Bounce2.h>

//Send signals as native MIDI device
#include "MIDIUSB.h"

//Watchdog library
#include <avr/wdt.h>


#define BUTTON_1_PIN 2
#define BUTTON_2_PIN 3
#define BUTTON_3_PIN 4
#define BUTTON_4_PIN 5

#define BUTTON_LED_1_PIN 6
#define BUTTON_LED_2_PIN 7
#define BUTTON_LED_3_PIN 8
#define BUTTON_LED_4_PIN 9

#define POTI_1_PIN A0
#define POTI_2_PIN A1
#define POTI_3_PIN A2
#define POTI_4_PIN A3



#define BUTTON_1_CONTROL 0x14
#define BUTTON_2_CONTROL 0x15
#define BUTTON_3_CONTROL 0x16
#define BUTTON_4_CONTROL 0x17

#define POTI_1_CONTROL 0x18
#define POTI_2_CONTROL 0x19
#define POTI_3_CONTROL 0x1A
#define POTI_4_CONTROL 0x1B



#define BUTTON_SETTING


#define MIDI_BUTTON_HIGH 127
#define MIDI_BUTTON_LOW 0


//The sum of all Buttons, Potis or else
#define MIDI_COMPONENTS 8

#define MIDI_CHANNEL 0


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



bool sendMidiMessages;

midiEventPacket_t midiMessages[MIDI_COMPONENTS] = {};

int midiMessageCounter = 0;

void setup() {
  Serial.begin(9600);
  while (!Serial); 
  

  pinMode(BUTTON_LED_1_PIN, OUTPUT);
  pinMode(BUTTON_LED_2_PIN, OUTPUT);
  pinMode(BUTTON_LED_3_PIN, OUTPUT);
  pinMode(BUTTON_LED_4_PIN, OUTPUT);

  //Light up all LEDs to test them
  digitalWrite(BUTTON_LED_1_PIN, HIGH); 
  delay(250);
  digitalWrite(BUTTON_LED_2_PIN, HIGH);
  delay(250);
  digitalWrite(BUTTON_LED_3_PIN, HIGH);
  delay(250);
  digitalWrite(BUTTON_LED_4_PIN, HIGH);
  delay(250);
  digitalWrite(BUTTON_LED_1_PIN, LOW);
  delay(250);
  digitalWrite(BUTTON_LED_2_PIN, LOW);
  delay(250);
  digitalWrite(BUTTON_LED_3_PIN, LOW);
  delay(250);
  digitalWrite(BUTTON_LED_4_PIN, LOW);

  Serial.println("Test");

  button1.attach(BUTTON_1_PIN, INPUT);
  button2.attach(BUTTON_2_PIN, INPUT);
  button3.attach(BUTTON_3_PIN, INPUT);
  button4.attach(BUTTON_4_PIN, INPUT);

  button1.interval(25);
  button2.interval(25);
  button3.interval(25);
  button4.interval(25);

  //wdt_enable( WDTO_2S);
}




void loop() {  
 
  potiValue1 = map(analogRead(POTI_1_PIN), 0, 1023, 0, 127);
  potiValue2 = map(analogRead(POTI_2_PIN), 0, 1023, 0, 127);
  potiValue3 = map(analogRead(POTI_3_PIN), 0, 1023, 0, 127);
  potiValue4 = map(analogRead(POTI_4_PIN), 0, 1023, 0, 127);

  button1.update();
  button2.update();
  button3.update();
  button4.update();


  button1Pressed = button1.rose();
  button2Pressed = button2.rose();
  button3Pressed = button3.rose();
  button4Pressed = button4.rose();
  
  sendMidiMessages = false;

  
  clearMidiPacketArray();


  if(button1Pressed) {
      int value;
      //Serial.println("Button 1: ");
      //erial.print(button1Pressed);
      if(button1State){
        button1State = false;
        value = MIDI_BUTTON_LOW;
      }else {
        button1State = true;
        value = MIDI_BUTTON_HIGH;
         
      }

    
      midiMessages[ midiMessageCounter ] = controlChange(MIDI_CHANNEL, BUTTON_1_CONTROL, value);
    
      sendMidiMessages = true;
      midiMessageCounter++;

      digitalWrite(BUTTON_LED_1_PIN, button1State);
    }
  if(button2Pressed) {

      int value;
      //Serial.println("Button 2: ");
      //Serial.print(button2Pressed);
      if(button2State){
        button2State = false;
        value = MIDI_BUTTON_LOW;
      }else {
        button2State = true;
        value = MIDI_BUTTON_HIGH;
         
      }

      
      midiMessages[ midiMessageCounter ] = controlChange(MIDI_CHANNEL, BUTTON_2_CONTROL, value);
    
      sendMidiMessages = true;
      midiMessageCounter++;

      
      digitalWrite(BUTTON_LED_2_PIN, button2State);
    }
  if(button3Pressed) {
      int value;
      
      //Serial.println("Button 3: ");
      //Serial.print(button3Pressed); 
      if(button3State){
        button3State = false;
        value = MIDI_BUTTON_LOW;
      }else {
        button3State = true;
        value = MIDI_BUTTON_HIGH;
         
      }
    
      midiMessages[ midiMessageCounter ] = controlChange(MIDI_CHANNEL, BUTTON_3_CONTROL, value);
    
      sendMidiMessages = true;
      midiMessageCounter++;

      
      digitalWrite(BUTTON_LED_3_PIN, button3State);
    }
  if(button4Pressed) {
      int value;
      //Serial.println("Button 4: ");
      //Serial.print(button4Pressed);
      if(button4State){
        button4State = false;
        value = MIDI_BUTTON_LOW;
      }else {
        button4State = true;
        value = MIDI_BUTTON_HIGH;
         
      }
    

    
      midiMessages[ midiMessageCounter ] = controlChange(MIDI_CHANNEL, BUTTON_4_CONTROL, value);
      
      sendMidiMessages = true;
      midiMessageCounter++;

      
      digitalWrite(BUTTON_LED_4_PIN, button4State);
    }



  
  
  if(currentValuePoti1 != potiValue1) {
      midiMessages[ midiMessageCounter ] = controlChange(MIDI_CHANNEL, POTI_1_CONTROL, potiValue1);
      //Serial.println("Poti 1: ");
      //Serial.print(potiValue1);
      sendMidiMessages = true;
      midiMessageCounter++;
      currentValuePoti1 = potiValue1;
    }
  
  if(currentValuePoti2 != potiValue2) {
      midiMessages[ midiMessageCounter ] = controlChange(MIDI_CHANNEL, POTI_2_CONTROL, potiValue2);
      //Serial.println("Poti 2: ");
      //Serial.print(potiValue2);
      sendMidiMessages = true;
      midiMessageCounter++;
      currentValuePoti2 = potiValue2;
    }
  
  if(currentValuePoti3 != potiValue3) {
      midiMessages[ midiMessageCounter ] = controlChange(MIDI_CHANNEL, POTI_3_CONTROL, potiValue3);
      //Serial.println("Poti 3: ");
      //Serial.print(potiValue3);
      sendMidiMessages = true;
      midiMessageCounter++;
      currentValuePoti3 = potiValue3;
    }
  
  if(currentValuePoti4 != potiValue4) {
      midiMessages[ midiMessageCounter ] = controlChange(MIDI_CHANNEL, POTI_4_CONTROL, potiValue4);
      //Serial.println("Poti 4: ");
      //Serial.print(potiValue4);
      sendMidiMessages = true;
      midiMessageCounter++;
      currentValuePoti4 = potiValue4;
    }
    
  


    
  
 if(sendMidiMessages){

  sendMidiPackages(midiMessages);
  MidiUSB.flush();
  
 }

 delay(100);
  //wdt_reset();
  
}





void sendMidiPackages ( midiEventPacket_t messages []){
  if(malloc(sizeof(messages)) != 0){
    for(int i = 0; malloc(sizeof(messages[i])) != 0   && i < MIDI_COMPONENTS ; i++){
      MidiUSB.sendMIDI(messages[i]);
    }
  }
}


// First parameter is the event type (0x09 = note on, 0x08 = note off).
// Second parameter is note-on/note-off, combined with the channel.
// Channel can be anything between 0-15. Typically reported to the user as 1-16.
// Third parameter is the note number (48 = middle C).
// Fourth parameter is the velocity (64 = normal, 127 = fastest).

void noteOn(byte channel, byte pitch, byte velocity) {
  midiEventPacket_t noteOn = {0x09, 0x90 | channel, pitch, velocity};
  MidiUSB.sendMIDI(noteOn);
}

void noteOff(byte channel, byte pitch, byte velocity) {
  midiEventPacket_t noteOff = {0x08, 0x80 | channel, pitch, velocity};
  MidiUSB.sendMIDI(noteOff);
}


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


void clearMidiPacketArray(){
  if(malloc(sizeof(midiMessages)) != 0){
      for(int i = 0; malloc(sizeof(midiMessages[i])) != 0 && i < MIDI_COMPONENTS ; i++) {
        //delete midiArray[i];
        midiMessages[i] = {};
      }
  }
}
