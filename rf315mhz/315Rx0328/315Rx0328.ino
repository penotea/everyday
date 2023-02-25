#include <RH_ASK.h>
#include "MIDIUSB.h"


#define ledPin 1 // ATTiny build in LED

// tx/rx configuration
#define rxPin 2 // ATTiny, RX on D3 (pin 2 on attiny85)
#define txPin 0 // ATTiny, TX on D1 (pin 0 on attiny85)
#define txSpeed 2000

RH_ASK driver(txSpeed, rxPin, txPin);

const char bellKey[] =  "1234"; // max 10 bytes

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
    uint8_t buf[10]  = {0}; 
    uint8_t buflen = sizeof(buf);
   
    if (driver.recv(buf, &buflen)) // Non-blocking
    {
      Serial.print("Message: ");
      Serial.println((char*)buf);
      noteOn(0, 36, 64);  //midi
      MidiUSB.flush();
      delay(500);  
      noteOff(0, 36, 64);
      MidiUSB.flush();
      if (strncmp((char*)buf, bellKey,strlen(bellKey)) == 0 ) {
        digitalWrite(ledPin, HIGH);   // turn the LED on (HIGH is the voltage level)
        noteOn(0, 36, 64);  //midi
        MidiUSB.flush();
        delay(1000);                       // wait for a second
        digitalWrite(ledPin, LOW);    // turn the LED off by making the voltage LOW
        delay(100);                       // wait for a second
        noteOff(0, 36, 64);
        MidiUSB.flush();
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
