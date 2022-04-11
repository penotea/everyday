#include <RH_ASK.h>
#include "MIDIUSB.h"

#define ledPin 9 // ATTiny build in LED

// tx/rx configuration
#define rxPin 2 // ATTiny, RX on D3 (pin 2 on attiny85)
#define txPin 0 // ATTiny, TX on D1 (pin 0 on attiny85)
#define txSpeed 2000

RH_ASK driver(txSpeed, rxPin, txPin);

const char Key_A[] =  "A"; // max 10 bytes
const char Key_B[] =  "B"; // max 10 bytes
const char Key_C[] =  "C"; // max 10 bytes
const char Key_D[] =  "D"; // max 10 bytes
const char Key_E[] =  "E"; // max 10 bytes
const char Key_F[] =  "F"; // max 10 bytes
const char Key_G[] =  "G"; // max 10 bytes
const char Key_H[] =  "H"; // max 10 bytes

void setup()
{
    Serial.begin(9600); // Debugging only
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
      //Serial.print("Message: ");
      //Serial.println((char*)buf);
      if(strncmp((char*)buf, Key_A,strlen(Key_A)) == 0){ //A
        //Serial.println("A");
        noteOn(3, 50, 64);
        MidiUSB.flush();
        digitalWrite(ledPin, HIGH);
        delay(100);
        
        noteOff(3, 50, 64);
        MidiUSB.flush();
        digitalWrite(ledPin, LOW);
      }
      if (strncmp((char*)buf, Key_B,strlen(Key_B)) == 0 ){ //B
        //Serial.println("B");
        noteOn(3, 48, 64);   // Channel 0, middle C, normal velocity
        MidiUSB.flush();
        digitalWrite(ledPin, HIGH);
        delay(100);
        
        noteOff(3, 48, 64);  // Channel 0, middle C, normal velocity
        MidiUSB.flush();
        digitalWrite(ledPin, LOW);
      }
      if(strncmp((char*)buf, Key_G,strlen(Key_G)) == 0){ //G
        Serial.println("G");
        noteOn(3, 43, 64);
        MidiUSB.flush();
        digitalWrite(ledPin, HIGH);
        delay(100);
        noteOff(3, 43, 64);
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
