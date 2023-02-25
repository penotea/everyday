#include <RH_ASK.h>
#include "MIDIUSB.h"

#define ledPin 9 // ATTiny build in LED

// tx/rx configuration
#define rxPin 2 // ATTiny, RX on D3 (pin 2 on attiny85)
#define txPin 0 // ATTiny, TX on D1 (pin 0 on attiny85)
#define txSpeed 4000

RH_ASK driver(txSpeed, rxPin, txPin);

const char Key_A[] =  "A";
const char Key_B[] =  "B";
const char Key_C[] =  "C";
const char Key_D[] =  "D";

const char Key_E[] =  "E";
const char Key_F[] =  "F";
const char Key_G[] =  "G";
const char Key_H[] =  "H";

const char Key_1[] =  "1";
const char Key_2[] =  "2";
const char Key_3[] =  "3";
const char Key_4[] =  "4";

const char Key_5[] =  "5";
const char Key_6[] =  "6";
const char Key_7[] =  "7";
const char Key_8[] =  "8";

const char Key_X[] = "X"; //clock

int nowclock,midiinterval;  //MIDIクロック関係
unsigned long prevtime,nexttime,intervaltime; //時間

void setup()
{
    Serial.begin(9600);
    if (!driver.init())
         Serial.println("init failed");
    else
         Serial.println("ready");
  
  pinMode(ledPin, OUTPUT);

  if (!driver.init()) {
    digitalWrite(ledPin, HIGH);   // turn the LED on (HIGH is the voltage level)
    delay(10000);                 // wait 10 seconds
  }
}

void loop()
{
    uint8_t buf[1]  = {0}; 
    uint8_t buflen = sizeof(buf);
    
    if (driver.recv(buf, &buflen)) // Non-blocking
    {
      Serial.print("Message: ");
      Serial.println((char*)buf);
      
      if(strncmp((char*)buf, Key_A,strlen(Key_A)) == 0){ //A
        Serial.println("A");
        noteOn(3, 63, 64);
        MidiUSB.flush();
        digitalWrite(ledPin, HIGH);
        delay(10);
        
        noteOff(3, 63, 64);
        MidiUSB.flush();
        digitalWrite(ledPin, LOW);
      }
      
      if (strncmp((char*)buf, Key_B,strlen(Key_B)) == 0 ){ //B
        Serial.println("B");
        noteOn(3, 68, 64);   // Channel 0, middle C, normal velocity
        MidiUSB.flush();
        digitalWrite(ledPin, HIGH);
        delay(10);
        
        noteOff(3, 68, 64);  // Channel 0, middle C, normal velocity
        MidiUSB.flush();
        digitalWrite(ledPin, LOW);
      }
      
      if (strncmp((char*)buf, Key_C,strlen(Key_C)) == 0 ){ //C
        Serial.println("C");
        noteOn(3, 56, 64);   // Channel 0, middle C, normal velocity
        MidiUSB.flush();
        digitalWrite(ledPin, HIGH);
        delay(10);
        
        noteOff(3, 56, 64);  // Channel 0, middle C, normal velocity
        MidiUSB.flush();
        digitalWrite(ledPin, LOW);
      }
      
      if (strncmp((char*)buf, Key_D,strlen(Key_D)) == 0 ){ //D
        Serial.println("D");
        noteOn(3, 64, 64);   // Channel 0, middle C, normal velocity
        MidiUSB.flush();
        digitalWrite(ledPin, HIGH);
        delay(10);
        
        noteOff(3, 64, 64);  // Channel 0, middle C, normal velocity
        MidiUSB.flush();
        digitalWrite(ledPin, LOW);
      }
      
      if (strncmp((char*)buf, Key_E,strlen(Key_E)) == 0 ){ //E
        Serial.println("E");
        noteOn(3, 41, 64);   // Channel 0, middle C, normal velocity
        MidiUSB.flush();
        digitalWrite(ledPin, HIGH);
        delay(10);
        
        noteOff(3, 41, 64);  // Channel 0, middle C, normal velocity
        MidiUSB.flush();
        digitalWrite(ledPin, LOW);
      }
      
      if (strncmp((char*)buf, Key_F,strlen(Key_F)) == 0 ){ //F
        Serial.println("F");
        noteOn(3, 42, 64);   // Channel 0, middle C, normal velocity
        MidiUSB.flush();
        digitalWrite(ledPin, HIGH);
        delay(10);
        
        noteOff(3, 42, 64);  // Channel 0, middle C, normal velocity
        MidiUSB.flush();
        digitalWrite(ledPin, LOW);
      }
      
      if(strncmp((char*)buf, Key_G,strlen(Key_G)) == 0){ //G
        Serial.println("G");
        noteOn(3, 43, 64);
        MidiUSB.flush();
        digitalWrite(ledPin, HIGH);
        delay(10);
        noteOff(3, 43, 64);
        MidiUSB.flush();
        digitalWrite(ledPin, LOW);
      }
      if(strncmp((char*)buf, Key_H,strlen(Key_H)) == 0){ //H
        Serial.println("H");
        noteOn(3, 44, 64);
        MidiUSB.flush();
        digitalWrite(ledPin, HIGH);
        delay(10);
        noteOff(3, 44, 64);
        MidiUSB.flush();
        digitalWrite(ledPin, LOW);
      }

      //1~8

      if(strncmp((char*)buf, Key_1,strlen(Key_1)) == 0){ //1
        Serial.println("1");
        noteOn(4, 36, 64);
        MidiUSB.flush();
        digitalWrite(ledPin, HIGH);
        delay(10);
        
        noteOff(4, 36, 64);
        MidiUSB.flush();
        digitalWrite(ledPin, LOW);
      }
      
      if (strncmp((char*)buf, Key_2,strlen(Key_2)) == 0 ){ //B
        Serial.println("2");
        noteOn(4, 37, 64);   // Channel 0, middle C, normal velocity
        MidiUSB.flush();
        digitalWrite(ledPin, HIGH);
        delay(10);
        
        noteOff(4, 37, 64);  // Channel 0, middle C, normal velocity
        MidiUSB.flush();
        digitalWrite(ledPin, LOW);
      }
      
      if (strncmp((char*)buf, Key_3,strlen(Key_3)) == 0 ){ //3
        Serial.println("3");
        noteOn(4, 38, 64);   // Channel 0, middle C, normal velocity
        MidiUSB.flush();
        digitalWrite(ledPin, HIGH);
        delay(10);
        
        noteOff(4, 38, 64);  // Channel 0, middle C, normal velocity
        MidiUSB.flush();
        digitalWrite(ledPin, LOW);
      }
      
      if (strncmp((char*)buf, Key_4,strlen(Key_4)) == 0 ){ //4
        Serial.println("4");
        noteOn(4, 39, 64);   // Channel 0, middle C, normal velocity
        MidiUSB.flush();
        digitalWrite(ledPin, HIGH);
        delay(10);
        
        noteOff(4, 39, 64);  // Channel 0, middle C, normal velocity
        MidiUSB.flush();
        digitalWrite(ledPin, LOW);
      }
      
      if (strncmp((char*)buf, Key_5,strlen(Key_5)) == 0 ){ //5
        Serial.println("5");
        noteOn(4, 40, 64);   // Channel 0, middle C, normal velocity
        MidiUSB.flush();
        digitalWrite(ledPin, HIGH);
        delay(10);
        
        noteOff(4, 40, 64);  // Channel 0, middle C, normal velocity
        MidiUSB.flush();
        digitalWrite(ledPin, LOW);
      }
      
      if (strncmp((char*)buf, Key_6,strlen(Key_6)) == 0 ){ //6
        Serial.println("6");
        noteOn(4, 41, 64);   // Channel 0, middle C, normal velocity
        MidiUSB.flush();
        digitalWrite(ledPin, HIGH);
        delay(10);
        
        noteOff(4, 41, 64);  // Channel 0, middle C, normal velocity
        MidiUSB.flush();
        digitalWrite(ledPin, LOW);
      }
      
      if(strncmp((char*)buf, Key_7,strlen(Key_7)) == 0){ //7
        Serial.println("7");
        noteOn(4, 42, 64);
        MidiUSB.flush();
        digitalWrite(ledPin, HIGH);
        delay(10);
        noteOff(4, 42, 64);
        MidiUSB.flush();
        digitalWrite(ledPin, LOW);
      }
      if(strncmp((char*)buf, Key_8,strlen(Key_8)) == 0){ //8
        Serial.println("8");
        noteOn(4, 43, 64);
        MidiUSB.flush();
        digitalWrite(ledPin, HIGH);
        delay(10);
        noteOff(4, 43, 64);
        MidiUSB.flush();
        digitalWrite(ledPin, LOW);
      }
    }
}


void noteOn(byte channel, byte pitch, byte velocity) {
  midiEventPacket_t noteOn = {0x09, 0x90 | channel, pitch, velocity};
  MidiUSB.sendMIDI(noteOn);
}

void noteOff(byte channel, byte pitch, byte velocity) {
  midiEventPacket_t noteOff = {0x08, 0x80 | channel, pitch, velocity};
  MidiUSB.sendMIDI(noteOff);
}

void controlChange(byte channel, byte control, byte value) {
  midiEventPacket_t event = {0x0B, 0xB0 | channel, control, value};
  MidiUSB.sendMIDI(event);
}

void MidiClock(){
  MidiUSB.sendMIDI({0x0F, 0xF8, 0, 0}); //clock
  MidiUSB.flush();
}

void MidiClockStart(){
  MidiUSB.sendMIDI({0x0F, 0xFA, 0, 0}); //clockstart
  MidiUSB.flush();
}

void MidiClockStop(){
  MidiUSB.sendMIDI({0x0F, 0xFC, 0, 0}); //clockstop
  MidiUSB.flush();
}
