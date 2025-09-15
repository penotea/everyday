#include <RH_ASK.h>
#include "MIDIUSB.h"

#define ledPin 9 // ATTiny build in LED

// tx/rx configuration
#define rxPin 2 // ATTiny, RX on D3 (pin 2 on attiny85)
#define txPin 0 // ATTiny, TX on D1 (pin 0 on attiny85)
#define txSpeed 4000

//ロータリーSW
#define RotarySW1 5
#define RotarySW2 6
#define RotarySW3 7
#define RotarySW4 8


RH_ASK driver(txSpeed, rxPin, txPin);

const char Key_A[] =  "A";
const char Key_B[] =  "B";
const char Key_C[] =  "C";
const char Key_D[] =  "D";

const char Key_E[] =  "E";
const char Key_F[] =  "F";
const char Key_G[] =  "G";
const char Key_H[] =  "H";

const char Key_I[] =  "I";
const char Key_J[] =  "J";
const char Key_K[] =  "K";
const char Key_L[] =  "L";

const char Key_a[] =  "a";
const char Key_b[] =  "b";
const char Key_c[] =  "c";
const char Key_d[] =  "d";

const char Key_e[] =  "e";
const char Key_f[] =  "f";
const char Key_g[] =  "g";
const char Key_h[] =  "h";
const char Key_i[] =  "i";
const char Key_j[] =  "j";

const char Key_X[] = "X"; //clock

int nowclock,midiinterval;  //MIDIクロック関係
unsigned long prevtime,nexttime,intervaltime; //時間

void setup()
{
  pinMode(RotarySW1, INPUT_PULLUP);
  pinMode(RotarySW2, INPUT_PULLUP);
  pinMode(RotarySW3, INPUT_PULLUP);
  pinMode(RotarySW4, INPUT_PULLUP);

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

      if(strncmp((char*)buf, Key_a,strlen(Key_a)) == 0){ //1
        Serial.println("1");
        noteOn(4, 36, 64);
        MidiUSB.flush();
        digitalWrite(ledPin, HIGH);
        delay(10);
        
        noteOff(4, 36, 64);
        MidiUSB.flush();
        digitalWrite(ledPin, LOW);
      }
      
      if (strncmp((char*)buf, Key_b,strlen(Key_b)) == 0 ){ //B
        Serial.println("2");
        noteOn(4, 37, 64);   // Channel 0, middle C, normal velocity
        MidiUSB.flush();
        digitalWrite(ledPin, HIGH);
        delay(10);
        
        noteOff(4, 37, 64);  // Channel 0, middle C, normal velocity
        MidiUSB.flush();
        digitalWrite(ledPin, LOW);
      }
      
      if (strncmp((char*)buf, Key_c,strlen(Key_c)) == 0 ){ //3
        Serial.println("3");
        noteOn(4, 38, 64);   // Channel 0, middle C, normal velocity
        MidiUSB.flush();
        digitalWrite(ledPin, HIGH);
        delay(10);
        
        noteOff(4, 38, 64);  // Channel 0, middle C, normal velocity
        MidiUSB.flush();
        digitalWrite(ledPin, LOW);
      }
      
      if (strncmp((char*)buf, Key_d,strlen(Key_d)) == 0 ){ //4
        Serial.println("4");
        noteOn(4, 39, 64);   // Channel 0, middle C, normal velocity
        MidiUSB.flush();
        digitalWrite(ledPin, HIGH);
        delay(10);
        
        noteOff(4, 39, 64);  // Channel 0, middle C, normal velocity
        MidiUSB.flush();
        digitalWrite(ledPin, LOW);
      }
      
      if (strncmp((char*)buf, Key_e,strlen(Key_e)) == 0 ){ //5
        Serial.println("5");
        noteOn(4, 40, 64);   // Channel 0, middle C, normal velocity
        MidiUSB.flush();
        digitalWrite(ledPin, HIGH);
        delay(10);
        
        noteOff(4, 40, 64);  // Channel 0, middle C, normal velocity
        MidiUSB.flush();
        digitalWrite(ledPin, LOW);
      }
      
      if (strncmp((char*)buf, Key_f,strlen(Key_f)) == 0 ){ //6
        Serial.println("6");
        noteOn(4, 41, 64);   // Channel 0, middle C, normal velocity
        MidiUSB.flush();
        digitalWrite(ledPin, HIGH);
        delay(10);
        
        noteOff(4, 41, 64);  // Channel 0, middle C, normal velocity
        MidiUSB.flush();
        digitalWrite(ledPin, LOW);
      }
      
      if(strncmp((char*)buf, Key_g,strlen(Key_g)) == 0){ //7
        Serial.println("7");
        noteOn(4, 42, 64);
        MidiUSB.flush();
        digitalWrite(ledPin, HIGH);
        delay(10);
        noteOff(4, 42, 64);
        MidiUSB.flush();
        digitalWrite(ledPin, LOW);
      }
      if(strncmp((char*)buf, Key_h,strlen(Key_h)) == 0){ //8
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

int RotarySW(){
  int s1, s2, s4, s8, state;
  char state_h[3];
  int SWNum;
   
  s1 = digitalRead(RotarySW1);
  s2 = digitalRead(RotarySW2);
  s4 = digitalRead(RotarySW3);
  s8 = digitalRead(RotarySW4);

  if(s1+s2+s4+s8==4)SWNum=0;
  else if(s2+s4+s8==3)SWNum=1;
  else if(s1+s4+s8==3)SWNum=2;
  else if(s4+s8==2&&s1+s2=0)SWNum=3;
  else if(s4+s8==2&&s1+s2=0)SWNum=4;
  else if(s1+s2+s8=3)SWNum=5;
  else if(s8+s1==2&&s2+s4=0)SWNum=6;
  else if(s8==1&&s1+s2+s4=0)SWNum=7;
  else if(s1+s2+s4==3)SWNum=8;
  else if(s2+s4==2&&s1+s3=0)SWNum=9;

  
  state = s8 * 8 + s4 * 4 + s2 * 2 + s1;
  sprintf(state_h, "%x", state);

  Serial.print("s1 = ");
  Serial.print(s1);  
  Serial.print(", s2 = ");
  Serial.print(s2);  
  Serial.print(", s4 = ");
  Serial.print(s4);  
  Serial.print(", s8 = ");
  Serial.print(s8);
  Serial.print(", state = ");
  Serial.print(state_h);
  Serial.print("(");
  Serial.print(state);
  Serial.println(")");

  return state_h;
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
