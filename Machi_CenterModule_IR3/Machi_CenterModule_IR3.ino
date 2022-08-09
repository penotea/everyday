//街センターモジュール
//受光&USBMIDI

//#include <IRremote.h>
#include <IRremote.hpp> //IRremote.hppが最新verなのでこちらで
#include "MIDIUSB.h"

int IRSensPin = 5;
int IR_RECEIVE_PIN = 2;
const int LedPin = 10;

bool flag=0;

//mills系
unsigned long startTime;  
unsigned long currentTime;
unsigned long checkTime;
const unsigned long period = 100; //led点灯時間

//IRrecv irrecv(IRSensPin); // 受信で使用するオブジェクトを作成 'irrecv'
//decode_results results;  // 受信情報の格納先を作成 'results'

void setup(){
  Serial.begin(115200);
  //irrecv.enableIRIn();     // 作成したオブジェクトで赤外線受信をスタート
  IrReceiver.begin(IR_RECEIVE_PIN, ENABLE_LED_FEEDBACK); // Start the receiver
  pinMode(LedPin,OUTPUT);
  startTime = millis();  //initial start time
  controlChange(0, 111, 70);
}

void loop() {
    currentTime = millis(); //時計
    
    if (IrReceiver.decode()&& flag ==0){ //受信したかどうかの確認 未受信＝0/受信＝1
    Serial.println(IrReceiver.decodedIRData.decodedRawData, HEX);
    //IrReceiver.printIRResultShort(&Serial); // optional use new print version
      
    //Serial.println(results.value, HEX); //16進表示
    //Serial.println(IrReceiver.decodedIRData.decodedRawData, HEX);
    //irrecv.resume();                    // .decode()の返り値をリセット
    IrReceiver.resume(); // Enable receiving of the next value
    
         if(IrReceiver.decodedIRData.decodedRawData ==0x1){noteOn(0, 37, 64);}   // Channel 0, middle C, normal velocity
    else if(IrReceiver.decodedIRData.decodedRawData ==0x2){noteOn(0, 38, 64);}   // Channel 0, middle C, normal velocity
    else if(IrReceiver.decodedIRData.decodedRawData ==0x3){noteOn(0, 39, 64);}  // Channel 0, middle C, normal velocity
    else if(IrReceiver.decodedIRData.decodedRawData ==0x4){noteOn(0, 40, 64);}   // Channel 0, middle C, normal velocity
    else if(IrReceiver.decodedIRData.decodedRawData ==0x5){noteOn(0, 41, 64);}   // Channel 0, middle C, normal velocity
    else if(IrReceiver.decodedIRData.decodedRawData ==0x6){noteOn(0, 42, 64);}   // Channel 0, middle C, normal velocity
    else {noteOn(0, 36, 64);}
    
    MidiUSB.flush();
    flag=1;
    checkTime = millis();
    }
    else{
    delay(50);
    noteOff(0, 35, 64);
    noteOff(0, 36, 64);  // Channel 0, middle C, normal velocity
    noteOff(0, 37, 64);  // Channel 0, middle C, normal velocity
    noteOff(0, 38, 64);
    noteOff(0, 39, 64);
    noteOff(0, 40, 64);
    noteOff(0, 41, 64);
    MidiUSB.flush();
    flag=0;
    }

  if(currentTime < checkTime+period){
    digitalWrite(LedPin,HIGH);
  }else{digitalWrite(LedPin,LOW);}

/*
  //Led点灯
  if(flag){
  if (currentTime - startTime >= period)//test whether the period has elapsed
    {
        digitalWrite(LedPin, !digitalRead(LedPin));//if so, change the state of the LED.
        startTime = currentTime;
    }
    }
*/
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
