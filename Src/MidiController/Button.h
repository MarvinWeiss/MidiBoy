#ifndef BUTTON_H
#define BUTTON_H


#include <Arduino.h>
//Library which debounces button signals
#include <Bounce2.h>


typedef struct {
  int buttonPin;
  //The Debounce Instance of a Button, representing the button itself
  Bounce bounce;
  //The Pin ID of LED connected to a Button
  int buttonLED;
  //Indicator if Button is currently Activated
  bool buttonState;

  //indicates if button is pressed at the moment
  bool buttonPressed;
  //Midi Control Address
  byte midiControl;
} Button;

#endif
