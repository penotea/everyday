#include <RH_ASK.h>
#include <SPI.h>  // Not actually used but needed to compile
#include "PhotoReflectorModule.h"

// 無線モジュールtx/rx configuration
#define rxPin 2  // ATTiny, RX on D3 (pin 2 on attiny85)
#define txPin 9  // ATTiny, TX on D1 (pin 0 on attiny85)
#define txSpeed 1000

RH_ASK driver(txSpeed, rxPin, txPin,false);

uint8_t msg[3];  // 送信するメッセージ

int sendrepeat=0; //繰り返し送る回数

//音色系
uint8_t msgA[] = {0,1,2};  // 送信するメッセージ
uint8_t msgB[] = {3,4,5};  // 送信するメッセージ
uint8_t msgC[] = {6,7,8};  // 送信するメッセージ

uint8_t msgD[] = {9,10,11};  // 送信するメッセージ

//パーカッション系
uint8_t msg1[] = {101,102,103};  // 送信するメッセージ
uint8_t msg2[] = {104,105,106};  // 送信するメッセージ
uint8_t msg3[] = {107,108,109};  // 送信するメッセージ
uint8_t msg4[] = {110,111,112};  // 送信するメッセージ

//led
//たぶんいらないかも確認(なくしたLED)
//#define ledPin 6

int brightness = 0;
int ledflag = 0;
unsigned long prevtime, nexttime;
unsigned long intervaltime = 2;  //ledの更新周期

//neopixel
#include <Adafruit_NeoPixel.h>
#define PIN 11  //promini=11,promicro=16
#define NUMPIXELS 1
Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);
#define DELAYVAL 500

//Servo Prominiだけにする
/*
#include <ServoTimer2.h>
#define ServoPin  13  //promini=13,promicro=15
ServoTimer2 servo;
int pos = 0;
*/

//DIPSwの配列
int DIPSw[4];
int SumDipSw; //DIPスイッチの全体の値

#define irLedPin 12  // for photo reflector's LED  promicro=14,promini=12

const int sensorNum = 3;
PhotoReflectorModule sensors[sensorNum];

void setup() {
  Serial.begin(9600);
  if (!driver.init())
    Serial.println("init failed");

  pixels.begin();  // INITIALIZE NeoPixel strip object (REQUIRED)

  //servo.attach(ServoPin);  //サーボ Prominiだけ

  //DIPSW
  pinMode(2, INPUT_PULLUP);
  pinMode(3, INPUT_PULLUP);
  pinMode(4, INPUT_PULLUP);
  pinMode(5, INPUT_PULLUP);
  
  DIPSwRecognize();

  //起動時LED点灯
  pixels.setPixelColor(0, pixels.Color(100, 100, 100));
  pixels.show();  // Send the updated pixel colors to the hardware.
  delay(1000);
  pixels.clear();

  // sensors初期化
  sensors[0].setup(12, A1);
  sensors[1].setup(10, A2); //ここだけ調子悪い
  sensors[2].setup(8, A3);
}

//メッセージ
void sendMsg(uint8_t msg) {
    for(int i = 0; i <= sendrepeat; i++){
        driver.send(&msg, sizeof(msg));
        driver.waitPacketSent();
    }
}

void loop() {
  DIPSwRecognize();
  unsigned long CountMillis = millis();

  //neopixelの処理
  if ((CountMillis - prevtime) >= intervaltime) {
    if (ledflag == 1) {
      brightness += 2;
      if (brightness > 255) ledflag = 2;
    }
    if (ledflag == 2) {
      brightness--;
      if (brightness <= 0) ledflag = 0;
    }
    prevtime += intervaltime;
  }

  //neopixel lights
  pixels.setPixelColor(0, pixels.Color(0, 0, brightness));
  pixels.show();  // Send the updated pixel colors to the hardware.
  if (brightness == 0) pixels.clear();

  //たぶんいらない昔のled
  //analogWrite(ledPin,brightness);

  sensorsUpdate();

  // センサーの結果を使う
  if (sensors[0].isTriggered()) {
    //サウンド処理
    //tone(13,440,100);
    /*
    for (int i = 0; i < 100; ++i) {
    digitalWrite(13, random(2));
    delayMicroseconds(1000);
    }
    digitalWrite(13,LOW);
    */

    //sendMsg(msg[0]);
    if (SumDipSw == 2000)sendMsg(msgA[0]);
    if (SumDipSw == 2100)sendMsg(msgB[0]);
    if (SumDipSw == 2110)sendMsg(msgC[0]);
    if (SumDipSw == 2111)sendMsg(msgD[0]);
    //if (SumDipSw == 1101)sendMsg(msgE[0]);
    //if (SumDipSw == 1110)sendMsg(msgF[0]);
    //if (SumDipSw == 1111)sendMsg(msgG[0]);

    if (SumDipSw == 1000)sendMsg(msg1[0]);
    if (SumDipSw == 1100)sendMsg(msg2[0]);
    if (SumDipSw == 1110)sendMsg(msg3[0]);
    if (SumDipSw == 1111)sendMsg(msg4[0]);
    //if (SumDipSw == 2100)sendMsg(msg5[0]);
    //if (SumDipSw == 2101)sendMsg(msg6[0]);
    //if (SumDipSw == 2110)sendMsg(msg7[0]);
    //if (SumDipSw == 2111)sendMsg(msg8[0]);
    ledflag = 1;  //led点滅のflagを1に
  }

  if (sensors[1].isTriggered()) {
    //サウンド処理
    //tone(13,880,100);

    //sendMsg(msg[1]);
    if (SumDipSw == 2000)sendMsg(msgA[1]);
    if (SumDipSw == 2100)sendMsg(msgB[1]);
    if (SumDipSw == 2110)sendMsg(msgC[1]);
    if (SumDipSw == 2111)sendMsg(msgD[1]);
    //if (SumDipSw == 1101)sendMsg(msgE[1]);
    //if (SumDipSw == 1110)sendMsg(msgF[1]);
    //if (SumDipSw == 1111)sendMsg(msgG[1]);

    if (SumDipSw == 1000)sendMsg(msg1[1]);
    if (SumDipSw == 1100)sendMsg(msg2[1]);
    if (SumDipSw == 1110)sendMsg(msg3[1]);
    if (SumDipSw == 1111)sendMsg(msg4[1]);
    //if (SumDipSw == 2100)sendMsg(msg5[1]);
    //if (SumDipSw == 2101)sendMsg(msg6[1]);
    //if (SumDipSw == 2110)sendMsg(msg7[1]);
    //if (SumDipSw == 2111)sendMsg(msg8[1]);
    ledflag = 1;  //led点滅のflagを1に
  }

  if (sensors[2].isTriggered()) {
    //サウンド処理
    //tone(13,1660,100);

    //sendMsg(msg[2]);
    if (SumDipSw == 2000)sendMsg(msgA[2]);
    if (SumDipSw == 2100)sendMsg(msgB[2]);
    if (SumDipSw == 2110)sendMsg(msgC[2]);
    if (SumDipSw == 2111)sendMsg(msgD[2]);
    //if (SumDipSw == 1101)sendMsg(msgE[0]);
    //if (SumDipSw == 1110)sendMsg(msgF[0]);
    //if (SumDipSw == 1111)sendMsg(msgG[0]);

    if (SumDipSw == 1000)sendMsg(msg1[2]);
    if (SumDipSw == 1100)sendMsg(msg2[2]);
    if (SumDipSw == 1110)sendMsg(msg3[2]);
    if (SumDipSw == 1111)sendMsg(msg4[2]);
    //if (SumDipSw == 2100)sendMsg(msg5[0]);
    //if (SumDipSw == 2101)sendMsg(msg6[0]);
    //if (SumDipSw == 2110)sendMsg(msg7[0]);
    //if (SumDipSw == 2111)sendMsg(msg8[0]);
    ledflag = 1;  //led点滅のflagを1に
  }

  //Serial.print(current);
  //Serial.print(' ');
  //Serial.println(sensorSum);
}

//DIPSW関数
void DIPSwRecognize() {
  DIPSw[0] = digitalRead(2);
  DIPSw[1] = digitalRead(3);
  DIPSw[2] = digitalRead(4);
  DIPSw[3] = digitalRead(5);

  if (DIPSw[0] == 0) DIPSw[0] = 2;  //計算上1000の値を1か2に

  SumDipSw = DIPSw[0] * 1000 + DIPSw[1] * 100 + DIPSw[2] * 10 + DIPSw[3];

}

//フォトリフレクターセンシング
void sensorsUpdate() {
  for (int i = 0; i < sensorNum; ++i) {
    sensors[i].readOff();
  }

  for (int i = 0; i < sensorNum; ++i) {
    sensors[i].ledOn();
  }

  delayMicroseconds(PhotoReflectorModule::sensorLatency);

  for (int i = 0; i < sensorNum; ++i) {
    sensors[i].readOn();
  }

  for (int i = 0; i < sensorNum; ++i) {
    sensors[i].ledOff();
  }
}
