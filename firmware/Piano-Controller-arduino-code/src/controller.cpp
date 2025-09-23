/****************************************************
* Holds all code written to handle controller
* setup related funtions
* **************************************************/
#include <Arduino.h>
#include "servos.h"
#include "controller.h"

 /*
void copyServoArrays(int* dir, int* trim, boolean highNotes){ //Select high or lowside notes and related arrays
  if(highNotes == true){
    copyIntArray(trim,servoDir,numServos); // use high servo direction values
    copyIntArray(servoHighTrim,servoTrim,numServos); //use the high trim values
    servoFullHome = servoHighFullHome;
    servoFlatHome = servoHighFlatHome;
  }
  else{
    copyIntArray(servoLowDir,servoDir,numServos); // use high servo direction values
    copyIntArray(servoLowTrim,servoTrim,numServos); //use the high trim values
    servoFullHome = servoLowFullHome;
    servoFlatHome = servoLowFlatHome;
   // delay(1000);
  }
}
*/

//Used to copy direction arrays
void copyByteArray(byte* src, byte* dst, byte len) {
    for (int i = 0; i < len; i++) {
        *dst++ = *src++;
    }
}

//Used to copy direction arrays
void copyIntArray(int* src, int* dst, byte len) {
    for (int i = 0; i < len; i++) {
        *dst++ = *src++;
    }
}

/*****
      Purpose: Switch a relay disconnecting power to the high note servos.
              This is to avoid overcurrent draw from PSU
      Parameter list:
              int rlPin     The relay control pin on the arduino
      Return value:
            void
*****/
void switchRelay(int rlPin)
{
      pinMode(rlPin, OUTPUT);
      digitalWrite(rlPin, LOW);
      delay(3000);    //wait for the other set of servos to start up to avoid overcurrent
      digitalWrite(rlPin,HIGH);
      return;
}
