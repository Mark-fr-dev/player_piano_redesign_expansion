#ifndef CONTROLLER_H
#define CONTROLLER_H
//#include "servos.h"

//if connected to 5V via 10k Resistor denotes that this micro is controlling high notes
//int noteSel = A1;

//control pin for relay used to charge capacitor bank
//int relayPin = A3;
//bool highNotes = false ; //high/low notes flag

/******************************************************
SETUP FUNCTION DEFINITIONS
 ******************************************************/
boolean highLowSelect(int);  //Select High or low side of the piano
void setArrayPointers(bool hl,int* d,int* t);
void switchRelay(int);
void copyByteArray(byte* src, byte* dst, byte len);
void copyIntArray(int* src, int* dst, byte len);
#endif
