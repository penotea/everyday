#include "MIDIUSB.h"

int PhotoReflectorPin = A1;
const int LedPin = 9;
int flag=0;

void setup() {
Serial.begin(115200);
pinMode(LedPin,OUTPUT);
}
void loop() {
int PhotoReflectorVal;
PhotoReflectorVal = analogRead(PhotoReflectorPin) ; //アナログ1番ピンからセンサ値を読み込み
Serial.println(PhotoReflectorVal) ; // シリアルモニターへ表示

if(PhotoReflectorVal<100 && flag ==0){
  digitalWrite(LedPin,HIGH);
  Serial.println("Sending note on");
  noteOn(0, 48, 64);   // Channel 0, middle C, normal velocity
  MidiUSB.flush();
  flag=1;
  }
  else if(PhotoReflectorVal>100){
  digitalWrite(LedPin,LOW);
  Serial.println("Sending note off");
  noteOff(0, 48, 64);  // Channel 0, middle C, normal velocity
  MidiUSB.flush();
  flag=0;
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
