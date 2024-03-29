#define IR_SEND_PIN 10
#include <IRremote.hpp>
int Senspin = A1;
int Sensflag=0;
const int LedPin = 9;

//mills系
unsigned long startTime;  
unsigned long currentTime;
unsigned long checkTime;
const unsigned long period = 100; //led点灯時間

void setup() {
  IrSender.begin();
  startTime = millis();  //initial start time
}

// Protocol=NEC Address=0x10 Command=0xF8 Raw-Data=0x7F8EF10 32 bits LSB first
uint16_t address = 0x10;
uint16_t command = 0xF8;

void loop() {
  currentTime = millis(); //時計
  int PhotoReflectorVal;
  PhotoReflectorVal = analogRead(Senspin) ; //アナログ1番ピンからセンサ値を読み込み
  
  if(PhotoReflectorVal<100 && Sensflag ==0){
  IrSender.sendNEC(address, command, 0);
  Sensflag=1;
  checkTime = millis();
  }else if(PhotoReflectorVal>100) {
    Sensflag=0;
  }

  if(currentTime < checkTime+period){
    digitalWrite(LedPin,HIGH);
  }else{digitalWrite(LedPin,LOW);}
  
}
