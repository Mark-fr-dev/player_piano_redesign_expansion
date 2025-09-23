#include <Arduino.h>
#include <Servo.h>
//#include <MIDI.h>
#include <EEPROM.h>
//#include <midi_RingBuffer.h>
//#include <midi_Namespace.h>
/*
26/12/2019
Still struggling to get this working well for the high note. I put some of the
setup code into functions and now it doesn't work at all. Grr
 */

//22/12/2019
/*
Adding a ringbuffer to store incoming data better
See http://fortyseveneffects.github.io/arduino_midi_library/a00064.html
 */
// 24/03/2019
/* Piano Player revision 1.0 using code from Mike Cook's
  book to speed up my MIDI handling

  Also changing some of the funtions to use pointers

  receives midi signals plays them to
  piano independent of high or low notes of the piano
 */

#include "pitches.h"
#include "servos.h"
#include "controller.h"
//#include "midihandling.h"
//#define DEBUG_SERVO
//#define DEBUG_HANDLES
//#define DEBUG_BUFF

//#define SETUPEEPROM
//#define DEBUG_MIDI
//-----IO Pins -------
//static const int high_led = A0;  // LED displaying a NoteOn received
static const int diag_led = A2;   //LED used for simple diagnostics
//static const int relay_pin = 16; //
//static const int del = 200; //delay
/*if connected to 5V via 10k Resistor denotes that this micro is controlling
    high notes
    If connected to 0V = low notes */
const static int note_sel  = 17;
//-----Servo variables -----
extern Servo my_servo[];  //An array of servos
extern byte servo_pin[]; //Servo Arduino pins
extern byte *ptr_to_trim;
bool piano_high_notes; //high or low side?
byte *ptr_wh_home;
byte *ptr_bl_home;
extern const int off_ang;
byte servo_num = 0;
// -----Midi variables -----
const static byte midi_highest_piano_note = 0x6E;  //This is the max midi piano note dec 108
const static byte midi_high_nt_ptch_offst = 0x41; // Lowest note code + 44 notes = decimal 65
const static byte midi_low_nt_ptch_offst  = 0x15; // Lowest midi piano note code = decimal 21
//MIDI_CREATE_INSTANCE(HardwareSerial, Serial3, midiSerial);
/*-------Note buffer for on and off-------*/
//typedef byte pitch_t;
//typedef byte vel_t;
typedef struct {
  byte pitch;
  byte velocity;
} note_t ;

#define SIZE_OF_BUFFER 32 // Space to store 16 notes SHOULD ALWAYS BE AN EVEN NUMBER
int circularBuff[SIZE_OF_BUFFER];
int bufferLength = 0;
int readIndex = 0;
int writeIndex = 0;

int count = 0;
int count2 = 0;
#define off_delay 3000
//-----Microphone variables -----
static const byte microph_ana = A1;  //Analogue output of microphone
static const byte calibrate_pin = A0; //pin to request calibration
static const byte microph_dig_int = 18;    //Digital output of microphone
volatile byte keyPressed;

byte incomingByte;
byte note;
byte velocity;

int statusLed = 13;   // select the pin for the LED

int action = 2; //0 =note off ; 1=note on ; 2= nada

//-----Function declarations-----
bool checkOut(void);
bool buff_is_empty(void);
bool write_buff(note_t note);
note_t read_buff(void);
void buff_clr(void);
//void write_buff(fifo_t note);
//fifo_t read_buff(void);
//void MyControlChange(byte channel, byte number, byte value);
////void MyHandleNoteOn(byte channel, byte pitch, byte velocity);
//void MyHandleNoteOff(byte channel, byte pitch, byte velocity) ;

void ISR_keyPress();
void FlashLED(byte flashes);
bool MustCal(void);
bool CheckHighLow(void);
void Calibrate(void);
void playNote(byte note, byte velocity);
//=======================================================================
void setup() {
  #ifdef DEBUG_SERVO
  Serial.begin(115200);

  Serial.println("-----Hello Arduino-----");

  #endif

  bool cal = false;
  //-----Setup Pins-----
  pinMode(diag_led,OUTPUT);
  //------Check calibrate pin
  //cal = MustCal();
  //-----Do we need to calibrate?------
  pinMode(calibrate_pin, INPUT_PULLUP);
  for(byte i = 0; i < 100; i++){ //check calibrate_pin for 2 seconds after startup
    bool cp = digitalRead(calibrate_pin); //switch pulls input down
    if(cp == LOW){
      cal = true;
      break;
    }
    delay(20);
  }
   //------Check High/Low side-----
  piano_high_notes = CheckHighLow();
  //------This Mask may make the servos jitter less -----

  //------set array pointers to the correct arrays (high/Low)
  setArrayPointers(piano_high_notes);

  //------Calibrate here-----
  if(cal == true){
    Calibrate();
  }
  //------Now we can attach the servos------
  //TIMSK0 = 0;
  servoAttach();   //Attach to servos here

  //If the EEPROM has not been initialised correctly, then run this
  #ifdef SETUPEEPROM
  for(byte i = 0; i < num_servos; i++){
    Serial.print("Servo: ");
    Serial.print(i);
    Serial.print("---Angle: ");
    Serial.println(int(*(ptr_to_trim + i)));
  }
  #endif
  //------Clear the note buffer------
  //void buff_clr(void);
  //------Setup MIDI-----
  /*midiSerial.begin(MIDI_CHANNEL_OMNI);
//  midiSerial.setInputChannel(1);
  //midiSerial.turnThruOff();
  midiSerial.setHandleControlChange(MyControlChange);
  midiSerial.setHandleNoteOn(MyHandleNoteOn);
  midiSerial.setHandleNoteOff(MyHandleNoteOff);
  Serial.println("\t--- Midi is set up ----");*/

  #ifdef SETUPEEPROM
  for(byte i = 0; i < num_servos; i++){
      Serial.print("EEPROM value: \t");
      Serial.print(i);
      Serial.print("\t is: \t");
      Serial.println(EEPROM.read(i));
  }
  #endif
    Serial3.begin(31250);
}
//=============================================================================
void loop() {
   //midiSerial.read();
   //checkOut();
   count++;
   if (Serial3.available() > 0) {
      //count = 0;
       // read the incoming byte:
       incomingByte = Serial3.read();
       //Serial.print("Serial 3 byte\t");
       //Serial.println(incomingByte);
       // wait for as status-byte, channel 1, note on or off
       if (incomingByte == 144) { // CH1 note on message starting starting
         note = Serial3.read();
         velocity = Serial3.read();
         playNote(note, velocity);
         return;
       } else if (incomingByte == 128) { //CH1 note off message starting
         note = Serial3.read();
         playNote(note,0);
         return;
       } else if (incomingByte == 255) { // control change message
         servoHome();
         return;
         //not implemented yet
       } else if ((action == 0) && (note == 0)) { // if we received a "note off", we wait for which note (databyte)
         note = incomingByte;
         playNote(note, 0);
         note = 0;
         velocity = 0;
         action = 2;//now waiting for another noteOn or noteOff
         return;
       } else if ((action == 1) && (note == 0))
       { // if we received a "note on", we wait for the note (databyte)
         note = incomingByte;
         return;
       } else if ((action == 1) && (note != 0))
       { // ...and then the velocity
         velocity = incomingByte;
         playNote(note, velocity);
         note = 0;
         velocity = 0;
         action = 2; // waiting for
         return;
       }
       else if ((action == 3) && (incomingByte == 123))
       {
         servoHome();
         action = 2;
         return;
       } else if (incomingByte == 176) { // control change message
         return;
         //not implemented yet
       } else if (incomingByte == 208) { // aftertouch message starting
         return;//not implemented yet
       } else if (incomingByte == 160) { // polypressure message starting
         return;//not implemented yet
       } else {
         return;//nada
       }
     }

     /*if(count == 10000)
     {
       count = 0;
       count2++;
       if(count2 == 1000){
       servoHome();
       count2 = 0;
      }

    }*/
}
void playNote(byte note, byte velocity) {
  //Serial.println("In PlayNote");
  /*int value = LOW;
  if (velocity > 10) {
    value = HIGH;
  } else {
    value = LOW;
  }*/
  if(!piano_high_notes){
      if (note > midi_low_nt_ptch_offst && note < midi_high_nt_ptch_offst) {
        servoWrite(note - midi_low_nt_ptch_offst, velocity);
        return;
    }else{
      return;
    }
  }else if (piano_high_notes){
    if(note >= midi_high_nt_ptch_offst && note < midi_highest_piano_note){
      servoWrite(note - midi_high_nt_ptch_offst, velocity);
      return;
      //Serial.println("Playing a high note");
    }else{
      return;
    }
  } else {return;}
}

/*****
  Purpose: Calibrate servos by pressing each key individually
  Parameter List:
        none
  Return value:
        none
 *****/
void Calibrate(void){
  #ifdef DEBUG_SERVO
  Serial.println("\tIn Calibrate()");
  #endif
  pinMode(microph_dig_int, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(microph_dig_int), ISR_keyPress, RISING);
    servoTrimInit();
    detachInterrupt(digitalPinToInterrupt(microph_dig_int));
    return;
}
/*****
  Purpose: Check if this controller plays the high octaves or low octaves
  Parameter List:
        none
  Return value:
        bool oct  true - high notes
                  false - low notes
 *****/
bool CheckHighLow(void){
  #ifdef DEBUG_SERVO
  Serial.println("\tIn CheckingHL()");
  #endif
  bool ret;
  pinMode(note_sel, INPUT_PULLUP);
  ret = (bool)digitalRead(note_sel);  //Select High or low side of the piano
  Serial.print("\tPlaying the ");
  if(ret)
    Serial.print("high ");
  else
    Serial.print("low ");
  Serial.println("notes");
  return ret;
}
/*****
  Purpose: Check if the Calibrate key has been pressed for 2 seconds after a restart
  Parameter List:
        none
  Return value:
        bool cal  true - calibrate
                  false - do nothing
 *****/
bool MustCal(void){
#ifdef DEBUG_SERVO
  Serial.println("\tIn MustCal()");
#endif
  //-----Do we need to calibrate?------
  pinMode(calibrate_pin, INPUT_PULLUP);
  for(byte i = 0; i < 100; i++){ //check calibrate_pin for 2 seconds after startup
    bool cp = digitalRead(calibrate_pin); //switch pulls input down
    if(cp == LOW){
      return true;
      break;
    }
    delay(20);
  }
  return false;
}
/*****
  Purpose: Check for a piano keypress via microphone.
            Sets keyPressed to HIGH
  Parameter List:
        none
  Return value:
        void
 *****/
void ISR_keyPress(){
  keyPressed = HIGH;
//  FlashLED(2);
}
/*****
  Purpose: Flash diagnostic LED 'flashes' number of times
  Parameter List:
        byte flashes  number of times that must be flashed
  Return value:
        void
 *****/
 void FlashLED(byte flashes)
 {
   for(int i =0 ; i < flashes ; i++)
   {
     digitalWrite(diag_led,HIGH);
     delay(200);
     digitalWrite(diag_led,LOW);
     delay(200);
   }
 }
/*****
 Purpose: Check midi control change commands. Panic or all notes off can
             be sent via this command
            For definition see midi_Defs.h
 Parameter List:
         byte channel  command channel
         byte number  what is the command number
         byte value   the command value - not used for control change
 Return value:
         void
 *****/
/*void MyControlChange(byte channel, byte number, byte value){
  switch(number){
    case 120: case 121: case 123:
      servoHome(); //send all servos home
      buff_clr(); //start the buffer again
      return;
    default:
      return;
  }
}*/
/*****
 Purpose: MyHandleNoteON is the function that will be called by the Midi Library
          when a MIDI NOTE ON message is received.
          This command will be buffered for later execution.
          For definition see midi_Defs.h
 Parameter List:
         byte channel  command channel
         byte pitch     which piano key?
         byte velocity  This should be zero for note off
 Return value:
         bool         indicating successful addition to the buffer
 *****/
/*void MyHandleNoteOn(byte channel, byte pitch, byte velocity) {
  #ifdef DEBUG_HANDLES
  Serial.print("\t\tON--Ch ");
  Serial.print(channel);
  Serial.print("\tptch ");
  Serial.print(pitch);
  Serial.print("\tvel ");
  Serial.println(velocity);
  #endif
  if(channel >= 4){ //piano should be received on these channels
    #ifdef DEBUG_HANDLES
    Serial.println("ON wrong chan");
    #endif
    return;
  }else if(pitch  >= midi_highest_piano_note){ //piano should be received on these channels
    #ifdef DEBUG_HANDLES
    Serial.println("ON note2high");
    #endif
    return;
  }else if(pitch  <  midi_low_nt_ptch_offst){   //not a note this piano can play
    #ifdef DEBUG_HANDLES
    Serial.print("ON note2low\t");
    Serial.println(pitch);
    #endif
    return;
  }else if(piano_high_notes && (pitch < midi_high_nt_ptch_offst)){ //not a note for the high controller
    #ifdef DEBUG_HANDLES
    Serial.println("ON note4lowKeys");
    #endif
    return;
  }else if(!piano_high_notes && (pitch >= midi_high_nt_ptch_offst)){
    #ifdef DEBUG_HANDLES
    Serial.println("ON note4highKeys");
    #endif
    return;
  }
  note_t this_note;
  this_note.pitch = pitch;
  this_note.velocity = velocity;
  #ifdef DEBUG_HANDLES
  Serial.print("N/On Buffwrite p ");
  Serial.print(pitch);
  Serial.print(" v ");
  Serial.println(velocity);
  #endif
  if(write_buff(this_note)){
      return;
  } else {
    Serial.println("Buff++"); //buffer is full
      return;
  }
}*/


/*****
 Purpose: MyHandleNoteOFF is the function that will be called by the Midi Library
          when a MIDI NOTE OFF message is received.
          This command will be buffered for later execution.
          For definition see midi_Defs.h
 Parameter List:
         byte channel  command channel
         byte pitch     which piano key?
         byte velocity  This should be zero for note off
 Return value:
         bool   indicating successful addition to the buffer
 *****/
/*void MyHandleNoteOff(byte channel, byte pitch, byte velocity) {
  #ifdef DEBUG_HANDLES
  Serial.print("\t\tOFF--Ch ");
  Serial.print(channel);
  Serial.print("\tptch ");
  Serial.print(pitch);
  Serial.print("\tvel ");
  Serial.println(velocity);
  #endif

  if(channel >= 4){ //piano should be received on these channels
    #ifdef DEBUG_HANDLES
    Serial.println("OFF wrong chan");
    #endif
    return;
  }
  else if(pitch  >= midi_highest_piano_note){ //piano should be received on these channels
    #ifdef DEBUG_HANDLES
    Serial.print("OFF note2high\t");
    Serial.println(pitch);
    #endif
    return;
  }
  else if(pitch  <  midi_low_nt_ptch_offst){   //not a note this piano can play
    #ifdef DEBUG_HANDLES
    Serial.print("OFF note2low\t");
    Serial.println(pitch);
    #endif
    return;
  }else if(piano_high_notes && (pitch < midi_high_nt_ptch_offst)){ //not a note for the high controller
    #ifdef DEBUG_HANDLES
    Serial.println("OFF note4lowKeys");
    #endif
    return;
  }else if(!piano_high_notes && (pitch >= midi_high_nt_ptch_offst)){
    #ifdef DEBUG_HANDLES
    Serial.println("OFF note4highKeys");
    #endif
    return;
  }
  note_t this_note;
  this_note.pitch = pitch;
  this_note.velocity = velocity;
  #ifdef DEBUG_HANDLES
  Serial.print("N/OFF Buffwrite p ");
  Serial.print(pitch);
  Serial.print(" v ");
  Serial.println(velocity);
  #endif
  if(write_buff(this_note)){
      return;
  } else {
    Serial.println("Buff++"); //buffer is full
      return;
  }
}*/
  /*****
 Purpose: Write to the Ringbuffer
 Parameter List:
         note_str note    This is a note, with the first component being the
                          pitch and the second being velocity
 Return value:
         bool     1 - success
                  0 - buffer full
 *****/
/*bool write_buff(note_t note)
{
  #ifdef DEBUG_BUFF
  Serial.print("BuffWrSt \t");
  Serial.print(bufferLength);
  #endif
  if(bufferLength == SIZE_OF_BUFFER){
    Serial.println("Full Buff");
    return 0;   //Fail: Buffer full
  }
  //Serial.println("In Buffwrite");
  circularBuff[writeIndex] = note.pitch;
  circularBuff[writeIndex + 1] = note.velocity;
  writeIndex = writeIndex + 2;
  bufferLength = bufferLength + 2;
  if(writeIndex == SIZE_OF_BUFFER){
    Serial.println("BuffWr_Wrap");
    writeIndex = 0;
  }
  #ifdef DEBUG_BUFF
  Serial.print("BuffWrEnd \t");
  Serial.println(bufferLength);
  #endif
  return 1;   //Success
}*/
/*****
 Purpose: Read from the Ringbuffer
 Parameter List:
         void
 Return value:
         note_str note    This is a note, with the first component being the
                          pitch and the second being velocity
 *****/
/*note_t read_buff(void)
{
  #ifdef DEBUG_BUFF
  Serial.print("BuffRdSt \t");
  Serial.println(bufferLength);
  #endif
  //Serial.println("In Buffead");
  note_t this_note;
  if(bufferLength == 0){ //should never get here
    Serial.println("Err: Buff 0");
    this_note.pitch = 0;
    this_note.velocity = 0;
    return this_note; //return dummy note
  }
  this_note.pitch = circularBuff[readIndex];
  this_note.velocity = circularBuff[readIndex+1];
  bufferLength = bufferLength - 2;
  readIndex = readIndex + 2;

  if(readIndex == SIZE_OF_BUFFER){ //wrap the buffer
    Serial.println("BuffRd_Wrap");
    readIndex = 0;
  }

  #ifdef DEBUG_BUFF
  Serial.print("\tpitch\t");
  Serial.print(this_note.pitch);
  Serial.print("\tvel\t");
  Serial.println(this_note.velocity);
  Serial.print("BuffRdEnd \t");
  Serial.println(bufferLength);
  #endif
  return this_note;
} */
/*****
  Purpose: Check if the circular buffer is empty
  Parameter List:
        void
  Return value:
        bool  1 - Buffer is empty
              0 - Buffer not empty
 *****/
 /*bool buff_is_empty(void){
  if(bufferLength == 0)
    return 1;
  else
    return 0;
 }*/
 /*****
   Purpose: Check if the circular buffer is empty
   Parameter List:
         void
   Return value:
         void
  *****/
 /*void buff_clr(void){
   bufferLength = 0;
   readIndex = 0;
   writeIndex = 0;
   return;
 }
*/
/*****
  Purpose: This function runs when we have no incoming info on the midiSerial
            stream. It checks the note buffer and writes any out standing
            note on or note off data to the servos
  Parameter List:
        void
  Return value:
        void
 *****/
/*
bool checkOut(void){
    if(buff_is_empty()) //nothing to send to servos
    {
        //Serial.println("No_bf");
        return 0;
    }
    note_t this_note = read_buff();
    byte temp_pitch;

    if(piano_high_notes)
    {
          if(this_note.pitch < midi_high_nt_ptch_offst){ //not a note for this controller
            Serial.println("ChkOut: N2Low");
            return 0;
          }else{ // this_note.pitch must be >= midi_high_nt_ptch_offst
              temp_pitch = (this_note.pitch - midi_high_nt_ptch_offst);
              if(servoWrite(temp_pitch, this_note.velocity)){
                #ifdef DEBUG_SERVO
                Serial.println("Sv_wr_OK");
                #endif
                return 1;
              }
              else{
                Serial.println("Sv_wr_err");
                return 0;
              }
          }
    }else{ //we have a low note
          if(this_note.pitch >= midi_high_nt_ptch_offst){ //not a note for this controller
              Serial.println("ChkOut: N2High");
              return 0;
          }else{
              temp_pitch = this_note.pitch - midi_low_nt_ptch_offst;
              if(servoWrite(temp_pitch, this_note.velocity)){
                #ifdef DEBUG_SERVO
                Serial.println("Sv_wr_OK");
                #endif
                return 1;
              }else{
                Serial.println("Sv_wr_err");
                return 0;
              }
          }
    }
  }*/
/* USED TO BE PART OF checkOut()
      if(pitch >= midi_high_nt_ptch_offst && piano_high_notes == true){
      //  Serial.println("OFF: High Note");
        this_note.pitch = pitch - midi_high_nt_ptch_offst;
        this_note.velocity = 0;
      //  servoWrite(this_note.pitch, this_note.velocity);//write_buff(this_note);
        //write_buff(this_note);
        return;
      }
      else if( pitch >= midi_low_nt_ptch_offst
              && pitch < midi_high_nt_ptch_offst
              && piano_high_notes == false){
        this_note.pitch = pitch - midi_low_nt_ptch_offst;
        this_note.velocity = 0;
        Serial.println("OFF: Low Note");
        //servoWrite(this_note.pitch, this_note.velocity);//write_buff(this_note);
        //write_buff(this_note);
        return;

    {
    note_t note = read_buff();
    servoWrite(note.pitch, note.velocity);
    return;
  }*/

/*****
  Purpose: This function runs when we have no incoming info on the midiSerial
            stream. It checks the note buffer and writes any out standing
            note on or note off data to the servos
  Parameter List:
        void
  Return value:
        void
 *****/
/*
void checkOut(void){
  static int place = 0; //static to stored from the last check
  int count  = 0;       //count how many places we have checked
  byte sv_num = 0;
  while ((storeNote[place] == 0) && (count < buffer_length)){ //look for next note
    place++;
    if(place >= buffer_length){
      place = 0;
    }
    count++;
    if(count == buffer_length) //nothing in the buffer
    {
    //  Serial.println("StoreNote buff empty");
      return;
    }
  }
  if(count < buffer_length){//found the next note to write
    if(piano_high_notes == true){ //Adjust for high/low side of piano
      sv_num = storeNote[place] - midi_high_nt_ptch_offst;
    } else {
      sv_num = storeNote[place] - midi_low_nt_ptch_offst;
    }
    servoWrite(sv_num, storeVel[place]);
      }
   bufferRemove(storeNote[place],storeVel[place]);      // remove from buffer
  }
*/


/*****
  Purpose:     Store current note and velocity, send this when we have time
  Parameter List:
        byte note     which note (servo number)
        byte vel      what is the velocity?
  Return value:
        int         0 success, 1 fail
 *****/
/*int bufferSave(byte note, byte vel){
  //search for a free space
  int place = 0;
  while(storeNote[place] !=(byte)0 && place < buffer_length){
    place++;
  }
  if(place < buffer_length){//found one
    storeNote[place] = note;
    storeVel[place] = vel;
    return 0;
  }else{
    return 1;
  }
}
*/
/*****
  Purpose: This function removes values from the buffer by writing 0 into
            the storeNote array
  Parameter List:
        byte note     the note to remove
        byte vel      the velocity the note was set to
  Return value:
        bool          return 0 for success
                      return 1 for failure
 *****/
 /*
 bool bufferRemove(byte note, byte vel){
   //search for note
   int place = 0;
   while(storeNote[place] != note && place < buffer_length){
     place++;
   }
   if(place < buffer_length){ //found it
     if(storeVel[place] != vel){
       return 1;  //something is wrong in the buffer or there are two of same?
     }else{
       storeNote[place] = 0; //clear this place in the buffer
       buffer_track--;
       return 0;
     }
   }
 }
*/
 //=============================================================================
 /*****
   Purpose: Check the midiSerial port for incoming bytes. This function is
             a state machine changing a state variable depending on what byte
             we receive
   Parameter List:
           void
   Return value:
           void
  *****/
 // void checkIn(){
 //
 //   //Serial.println("\nCheckIn ");
 //   static byte note_num = 40;      //servo number related to the note received from midi
 //   if(midiSerial.available() > 0){
 //     time_past = millis();  //update time_past to now, for
 //         //read incoming byte
 //     byte incoming_byte = midiSerial.read();
 //     //midiSerial.write(incoming_byte); //send to thru
 //     switch (state) {
 //       case 0:
 //           // #ifdef DEBUG_MIDI
 //           //   Serial.print("\nIn CheckIn State: ");
 //           //   Serial.println(state);
 //           // #endif
 //         //look for status byte, our channel, note on
 //         if(incoming_byte == (note_on_message | channel1)||
 //                               (note_on_message | channel2)){
 //           note_down = HIGH;
 //           state = 1;
 //           #ifdef DEBUG_MIDI
 //             Serial.println("\n===============================");
 //             Serial.print("NoteOn Message");
 //           #endif
 //         }
 //         //look for a status byte, our channel, note on
 //         if(incoming_byte == ((note_off_message | channel1)||
 //                               (note_off_message | channel2))){
 //           note_down = LOW;
 //           state = 1;
 //           #ifdef DEBUG_MIDI
 //             Serial.println("\n===============================");
 //             Serial.print("NoteOff Message");
 //           #endif
 //         }
 //         //look for aftertouch on our channel...ignore for now
 //         if(incoming_byte == ((aftertouch_message | channel1)||
 //                               (aftertouch_message | channel1))){
 //           state = 3;
 //         }
 //         break;
 //       case 1:
 //         //get the note to play or stop
 //         if(piano_high_notes == true){     //are we playing high notes?
 //           Serial.println("Playing high notes");
 //           if(incoming_byte >= midi_high_nt_ptch_offst &&
 //               incoming_byte <= midi_highest_piano_note){
 //               note_num = incoming_byte - midi_high_nt_ptch_offst; //Servo 0 -43
 //               state = 2;
 //               #ifdef DEBUG_MIDI
 //                 Serial.print("\nNote Number: ");
 //                 Serial.println(note_num);
 //               #endif
 //           }else{//Don't care about this note
 //             state = 0;
 //           }
 //         }else{     //we must be playing the low notes
 //           if(midi_low_nt_ptch_offst <= incoming_byte &&
 //                     incoming_byte < midi_high_nt_ptch_offst){
 //             note_num = incoming_byte - midi_low_nt_ptch_offst;  //servo 0 - 43
 //             state = 2;
 //           }else{
 //             state = 0;
 //           }
 //           break;
 //         case 2:
 //             #ifdef DEBUG_MIDI
 //               Serial.print("Midi velocity: ");
 //               Serial.println(incoming_byte);
 //             #endif
 //         //get the velocity
 //           if(incoming_byte == 0 || note_down == LOW){ //switch off immediately
 //             #ifdef DEBUG_MIDI
 //               Serial.println("Send servo to off angle");
 //             #endif
 //             servoWrite(note_num, off_ang, my_servo,piano_high_notes);
 //           } else if(incoming_byte > 0 && incoming_byte < midi_max_vel){  //save to turn on later
 //               Serial.println("Send servo to on angle");
 //             servoWrite(note_num, incoming_byte, my_servo,piano_high_notes);
 //           }
 //           state = 0;
 //           break;
 //         //aftertouch data, reset state machine
 //         case 3:
 //             #ifdef DEBUG_MIDI
 //               Serial.print("\nAftertouch byte, State: ");
 //               Serial.println(state);
 //             #endif
 //           state = 0;
 //           break;
 //       }
 //     }
 //   }
   // static int i = 0;
   // i++;
   // if(i == 100) //check every 100 loops
   // {
   //   if(time_past + off_delay >= millis()) //check if we have waited more than off_delay
   //   {
   //     servoHome(off_ang,my_servo,piano_high_notes); //Send all servos to home position
   //     Serial.println("Turning all servos off after off_delay ");
   //   }
   //    i = 0;
   // }
 //}
