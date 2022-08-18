//HOUSE MUSIC
//

#include <RH_ASK.h>
#include <SPI.h>
// tx/rx configuration
#define rxPin 2 // ATTiny, RX on D3 (pin 2 on attiny85)
#define txPin 9 // ATTiny, TX on D1 (pin 0 on attiny85)
#define txSpeed 2000
//315MHZのドライバ
RH_ASK driver(txSpeed, rxPin, txPin);

//送信メッセージ
const char *Txmsg = "B";

//ホールセンサ
bool state=0;
int holeThrethold=900;

void setup(){
  pinMode(2,INPUT_PULLUP);
  pinMode(10,OUTPUT);
  Serial.begin(9600);             //9600bpsでシリアルポートを開く
  if (!driver.init())
      Serial.println("init failed");
}

void loop() {
  if(analogRead(A0)>holeThrethold && state==0){
    state=1;
    Tx315MHZ();
  }
  else if(analogRead(A0)<50){
    state=0;
    }
    
    Serial.println(state);
}

void Tx315MHZ(){
    driver.send((uint8_t *)Txmsg, strlen(Txmsg));
    driver.waitPacketSent();
    digitalWrite(13,HIGH);
    delay(50);
    digitalWrite(13,LOW);
  
}
