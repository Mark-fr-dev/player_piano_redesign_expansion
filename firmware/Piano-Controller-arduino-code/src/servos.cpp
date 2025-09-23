/*
This file holds all the servo related function written
for the Piano Player
 */
#include <Arduino.h>
#include <Servo.h>
#include <EEPROM.h>
#include "servos.h"

//#define SETUPEEPROM
//#define DEBUG_SERVO

Servo my_servo[num_servos];  //An array of servos
#ifdef SETUPEEPROM
/****** HIGH SIDE RESTART TRIM *******/
byte servo_trim_h[] = {140,125,120,135,115,130,115,122, //0 -- 7
                    130,126,130,125,125,120,120,130, // 8 -- 15
                    120,130,125,125,135,138,120,125, //16 -- 23
                    120,140,130,120,130,105,120,120, //24 - 31
                    105,120,130,100,130,130,120,120, // 32 -- 39
                    120,140,115,143
                    }; // 40 - 43
/*******LOW SIDE RESTART EEPROM TRIM ********/
byte servo_trim_l[] = {145,120,140,140,125,130,135,140, //0 -- 7
                      140,130,130,100,130,100,135,130, // 8 -- 15
                      100,125,120,130,125,100,130,110, //16 -- 23
                      120,130,125,140,110,130,100,115, //24 - 31
                      135,100,125,105,135,110,125,125, // 32 -- 39
                      110,125,115,150}; // 40 - 43

#else
byte servo_trim_h[num_servos];
byte servo_trim_l[num_servos];
#endif

//Pointers to arrays
byte *ptr_to_trim;
byte *ptr_to_dir;
const byte *ptr_to_wh_hm;
const byte *ptr_to_bl_hm;
extern volatile byte keyPressed;

/*****
Arduino pins matched to servos
*****/
byte servo_pin[] = { 2,  3,  4,  5,  6,  7,  8,  9,
                    10, 11, 12, 13, 19, 20, 21, 22,
                    30, 29, 28, 27, 26, 25, 24, 23,
                    38, 37, 36, 35, 34, 33, 32, 31,
                    46, 45, 44, 43, 42, 41, 40, 39,
                    53, 52, 51, 50, 49, 48, 47};
 /*****
   Purpose: To Check the trim of the a servo by listening for a keypress using
            a connected microphone.
   Parameter List:
         void
   Return value:
         void
   NOTE: Already run setArrayPointers() so that accounts for high or low notes.
  *****/
void servoTrimInit(void){
  #ifdef DEBUG_SERVO
    Serial.println("Initialising servos");
  #endif
  byte this_trim;
  byte old_trim;
  for(byte i = 0; i < num_servos; i++)
  {
    this_trim = *(ptr_to_trim + i);
    old_trim = this_trim;
    #ifdef DEBUG_SERVO
      Serial.print("Servo: ");
      Serial.print(i);
      Serial.print("bef trim: ");
      Serial.print(this_trim);
    #endif
    my_servo[i].attach(servo_pin[i]);    //attach to current servo
  //  my_servo[i].write(this_trim);
    keyPressed = LOW;  //reset for next interrupt
    //if(*(ptr_to_dir + i) == 1){ //Playing a white note, to press key --angle
    //  this_trim = this_trim + 15;
      if(this_trim > 170) this_trim = 170;
      while (keyPressed == LOW){
         #ifdef DEBUG_SERVO
           Serial.print("Keypress while loop servo ");
           Serial.print(i);
           Serial.print( " ");
           Serial.println(this_trim);
         #endif
        this_trim--;    //increment servo 1 deg until a note sounds
        if(this_trim == 0 || (this_trim == (old_trim - 65)))
          this_trim = old_trim + 20; //try again
        my_servo[i].write(this_trim);
        delay(30);
        }
        this_trim = this_trim - soft_angle; //retract servo by the softpress angle, but trim + soft_angle should make a note.
        my_servo[i].write(this_trim + 10); //this is to avoid the whole device inching upwards through the calibration
        delay(500);
        my_servo[i].detach();
    //} else if(*(ptr_to_dir + i) == 0){ //Playing a black note, to press a key ++ angle
      //  this_trim = this_trim;
      /*  if(this_trim < 10) this_trim = 10;
        while (keyPressed == LOW){
          // #ifdef DEBUG_SERVO
          //   Serial.print("Black while loop");
          //   Serial.println(this_trim);
          // #endif
          this_trim++;    //increment servo 1 deg until a note sounds
          my_servo[i].write(this_trim);
          delay(20);
        }
          this_trim = this_trim - 20; //retract servo by 15 degrees

      }*/
      #ifdef DEBUG_SERVO
        Serial.print("\tafter: ");
        Serial.println(this_trim);
      #endif
      *(ptr_to_trim + i) = this_trim;
      EEPROM.update(i, this_trim);
      //delay(1000);

    }
}
 /*****
       Purpose: Copy EEPROM trim values into sram.
                Point the controller to the correct arrays of trim and direction
               dependant on high notes or low notes
       Parameter list:
               bool hNotes    Is this the high note controller?
               int *ptr_t     Pointer to the 0th value of the trim array
               int *ptr_d     Pointer to the 0th value of the direction array
       Return value:
             void
 *****/
 void setArrayPointers(boolean h_notes)
 {
   if(h_notes)
   {
     ptr_to_trim = servo_trim_h;
   }else{
     ptr_to_trim = servo_trim_l;
   }

   #ifdef DEBUG_SERVO
   Serial.print("\nin setArrayPointers ");
   Serial.println(h_notes);
   #endif

      #ifdef SETUPEEPROM
      for(byte i = 0; i < num_servos; i++){
        EEPROM.update(i, byte(*(ptr_to_trim + i)));
        #ifdef DEBUG_SERVO
        Serial.print("S");
        Serial.print(i);
        Serial.print(" a-");
        Serial.println(int(*(ptr_to_trim + i)));
        #endif
      }
      #endif
      Serial.println("Reading from EEPROM");
      #ifndef SETUPEEPROM
       for(byte i = 0; i < num_servos; i++){
        if(h_notes){
          servo_trim_h[i] = EEPROM.read(i);
          #ifdef DEBUG_SERVO
          Serial.print("Servo number ");
          Serial.print(i);
          Serial.print(" angle is ");
          Serial.println(servo_trim_h[i]);
          #endif
        }else{
          servo_trim_l[i] = EEPROM.read(i);
          #ifdef DEBUG_SERVO
          Serial.print("Servo number ");
          Serial.print(i);
          Serial.print(" angle is ");
          Serial.println(servo_trim_l[i]);
          #endif
        }


       }
       #endif



    /*  if(h_notes == true){
        ptr_to_dir   = servo_high_dir;
      } else{
        ptr_to_dir   = servo_low_dir;
      }
      */

 }
/*****
      Purpose: Attach to all servos in the Servo array send them to the Home
                position
      Parameter list:
              void
      Return value:
            void
*****/
void servoAttach(void){
   for(byte i = 0; i < num_servos; i++){
      my_servo[i].attach(servo_pin[i]);
      my_servo[i].write((*(ptr_to_trim + i)) + retract_angle);
      delay(30); //not sure if the servo library waits for the servo to finish moving
  }
}
/*****
  Purpose: Converts velocity to one of three servo position angles, min_angle,
            med or max
  Parameter List:
        int vel     Velocity take from midiSerial
  Return value:
        int        Servo angle
 *****/
byte servoVelToAngle(byte sv_num, int vel){
      if (vel == 0){
          return (*(ptr_to_trim + sv_num)) + retract_angle; // return servo home
      } else if (vel == 1){
        return (*(ptr_to_trim + sv_num)) + hard_angle;

               // else if((vel > 0) && (vel <= soft_vel)){
               //     return *(ptr_to_trim + sv_num) + soft_angle; //soft press
               // } else if((vel > soft_vel) && (vel <= med_vel)){
               //     return *(ptr_to_trim + sv_num) + med_angle; //medium press
               // } else if (vel > med_vel && vel <= max_vel){
               //     return *(ptr_to_trim + sv_num) + hard_angle; //hard press
               // }
        } else if ((vel < 0) || (vel > max_vel))
        { // Should not get here, not valid
          Serial.print("Inv vel: ");
          Serial.println(vel);
          return 0;
        }
  return 0;
}
/*****
  Purpose: Write to a single servo, taking onto account white or black notes etc
  Parameter List:
        int sv_num      which servo?
        int velocity    what is the midi velocity?
        Servo *ptr_sv   pointer to the servo array
  Return value:
        bool    1 - successful
                0 - failed
 *****/
bool servoWrite(byte sv_num, byte velocity){
    if(sv_num >= 0 || sv_num < num_servos){
        byte angle = servoVelToAngle(sv_num, velocity);
        if(angle != 0){
          my_servo[sv_num].write(angle);
         // delay(1); // give a little more time to move the servo
          #ifdef DEBUG_SERVO
            Serial.print("Wr2Svo:" ); //writing to servo number
            Serial.println(sv_num);
            Serial.print(" servo angle: " );
            Serial.println(angle);
            #endif
          return 1;
        } else {
          Serial.println("Err!0Ang");
          return 0;
        }
    }
    else{
      Serial.println("Err!svnum");
      return 0;
    }
}

/*****
  Purpose: Sends a single servo home/to the off position
  Parameter List:
        int off_ang     The off angle for this servo
        int sv_num      Which servo?
        Servo *ptr_sv   pointer to the servo array
  Return value:
        void
 *****/
void servoHome(int sv_num){
  #ifdef DEBUG_SERVO
    Serial.println("In ServoHome single" );
    #endif
  //servoWrite(sv_num, *(ptr_to_trim + sv_num)); 
  servoWrite(sv_num,0);
}
/*****
  Purpose: Sends all the servos home
  Parameter List:
            int off_ang     The off angle for this servo
            Servo *ptr_sv   pointer to the servo array
  Return value:
        void
 *****/
void servoHome(void){
  #ifdef DEBUG_SERVO
    Serial.println("In ServoHome multi" );
    #endif
  for(int i = 0; i < num_servos ; i++){
    servoWrite(i,0);
    //my_servo[i].write(*(ptr_to_trim + i ));
  }
}
