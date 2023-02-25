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
  Serial.begin(115200);
}

// Protocol=NEC Address=0x10 Command=0xF8 Raw-Data=0x7F8EF10 32 bits LSB first
//uint16_t address = 0x10;
//uint16_t command = 0xF8;
uint16_t address = 0x11;
uint16_t command = 0x11;

void loop() {

  for(int i=0;i<20;i++){
  IrSender.sendNECRaw(i, 0);

  delay(1000);
  }
}
