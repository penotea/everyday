#include <RH_ASK.h>
#include <SPI.h>  // Not actually used but needed to compile
#include "PhotoReflectorModule.h"

// 無線モジュールtx/rx configuration
#define rxPin 2  // ATTiny, RX on D3 (pin 2 on attiny85)
#define txPin 9  // ATTiny, TX on D1 (pin 0 on attiny85)
#define txSpeed 4000

RH_ASK driver(txSpeed, rxPin, txPin);

char* msg = "A";  // 送信するメッセージ

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
  sensors[1].setup(10, A2);
  sensors[2].setup(8, A3);
}

void sendMsg(char* msg) {
  driver.send((uint8_t*)msg, strlen(msg));
  driver.waitPacketSent();
  //Serial.print("sent ");
  //Serial.println(msg);
  //digitalWrite(10,HIGH);
  //delay(200);
  //digitalWrite(10,LOW);
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


  //Serial.print(current);
  //Serial.print(' ');
  //Serial.println(sensorSum);
}


//サーボ回転関数
/*
void RotateServo(int ServoSpeed,int ServoPos){
  for (pos = 0; pos <= ServoPos; pos += 1) {  //ServoPos分回転
    //myservo.write(pos);
    servo.write(pos);
    delay(ServoSpeed);  //デフォルトは15
  }
}
*/

//DIPSW関数
void DIPSwRecognize() {
  DIPSw[0] = !digitalRead(2);
  DIPSw[1] = !digitalRead(3);
  DIPSw[2] = !digitalRead(4);
  DIPSw[3] = !digitalRead(5);

  if (DIPSw[0] == 0) DIPSw[0] = 2;  //計算上1000の値を1か2に

  int SumDipSw = DIPSw[0] * 1000 + DIPSw[1] * 100 + DIPSw[2] * 10 + DIPSw[3];
  //Serial.print("DipSw:");
  //Serial.println(SumDipSw);

  if (SumDipSw == 1000) msg = "A";
  if (SumDipSw == 1001) msg = "B";
  if (SumDipSw == 1010) msg = "C";
  if (SumDipSw == 1011) msg = "D";
  if (SumDipSw == 1100) msg = "E";
  if (SumDipSw == 1101) msg = "F";
  if (SumDipSw == 1110) msg = "G";
  if (SumDipSw == 1111) msg = "H";

  if (SumDipSw == 2000) msg = "1";
  if (SumDipSw == 2001) msg = "2";
  if (SumDipSw == 2010) msg = "3";
  if (SumDipSw == 2011) msg = "4";
  if (SumDipSw == 2100) msg = "5";
  if (SumDipSw == 2101) msg = "6";
  if (SumDipSw == 2110) msg = "7";
  if (SumDipSw == 2111) msg = "8";
}

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

  sensors[0].isTriggered();
  if (sensors[0].isTriggered()) {
    sendMsg(msg);
    ledflag = 1;  //led点滅のflagを1に
  }
}