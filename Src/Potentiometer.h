#ifndef POTENTIOMETER_H
#define POTENTIOMETER_H


#include <Arduino.h>
//Library which debounces button signals
#include <Bounce2.h>


typedef struct {
  //Pin ID of a Poti
  int potiPin;
  //value of Poti
  int potiValue;
  int newPotiValue;

  //Midi Control Address
  byte midiControl;

} Poti;

#endif
