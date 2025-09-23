#ifndef SERVOS_H
#define SERVOS_H

#include <Arduino.h>
#include <Servo.h>

/******************************************************
Function declarations related to servo setup and control
******************************************************/
void servoAttach(void);
void servoTrimInit(void);
bool isWhiteNote(byte *ptr_sv_dir, int sv_num);
//void servosHome(void);
//int servoTrim(void);
byte servoVelToAngle(int vel, bool is_white);
bool servoWrite(byte sv_num, byte velocity);
//void servoWrite(int note, int velocity, Servo *ptr_sv);
//void servosHome(void);
//void myWriteServo(Servo*, int, int );
void servoHome(int sv_num);
void servoHome(void);
//void servoHome(int off_ang, Servo *ptr_sv); //for all servos
void setArrayPointers(bool);
int servoPosition(int angle, int sv_num);

//Angles for respective velocities
static const byte soft_vel = 60; //maximum "soft" value 128/3
static const byte med_vel  = 90; //maximum "medium" value 128/3*2
static const byte max_vel = 127;   //maximum midi velocity

static const byte soft_angle = -15;
static const byte med_angle = -16;
static const byte hard_angle = -17;
static const byte retract_angle = 0;
//The number of servos we need to control 44 max
static const int num_servos =44;
#endif
