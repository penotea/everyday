#include <RH_ASK.h>
#include <SPI.h> // Not actually used but needed to compile

// tx/rx configuration
#define rxPin 2 // ATTiny, RX on D3 (pin 2 on attiny85)
#define txPin 9 // ATTiny, TX on D1 (pin 0 on attiny85)
#define irLedPin 14 // for photo reflector's LED
#define txSpeed 4000

RH_ASK driver(txSpeed, rxPin, txPin);

char * msg = "63"; // 送信するメッセージ、後でEEPROMから読むように変更

// tettouアルゴリズム
static const uint16_t historyLength = 30; //ここでオン・オフともに差分をためる分の長さ
int32_t history[historyLength] = {0};
uint32_t historyIndex = 0;
int32_t sensorSum = 0;//センサーオフのときの足し算したやつ
int32_t current = 0;
int32_t thresholdOnce = 4;  //ここで感度変えられそう(-分の差分)
int32_t threshold = thresholdOnce * historyLength;
bool pastSensorFlag = false;
uint32_t sensorLatency = 30; // センサーの応答特性（micro sec）

void setup()
{
    pinMode(2,INPUT_PULLUP);
    pinMode(10,OUTPUT);
    digitalWrite(10,LOW);
    Serial.begin(9600);
    if (!driver.init())
         Serial.println("init failed");

    pinMode(irLedPin, OUTPUT);
    digitalWrite(irLedPin, HIGH);
}

void sendMsg(char* msg) {
    driver.send((uint8_t *)msg, strlen(msg));
    driver.waitPacketSent();
    Serial.print("sent ");
    Serial.println(msg);
    digitalWrite(10,HIGH);
    delay(200);
    digitalWrite(10,LOW);
}

void loop()
{
  digitalWrite(irLedPin, LOW);
  delayMicroseconds(sensorLatency);
  current = analogRead(A1); // オフのときの値を取る
  digitalWrite(irLedPin, HIGH);
  delayMicroseconds(sensorLatency);
  current -= analogRead(A1); // オンのときで差分を取る（反射物があるときが正の値になる）

  sensorSum += current;
  sensorSum -= history[historyIndex];
  history[historyIndex] = current;
  historyIndex++;
  if (historyIndex >= historyLength) historyIndex = 0;
    
  bool currentSensorFlag = sensorSum >= threshold;
  if (currentSensorFlag && !pastSensorFlag) {
    sendMsg(msg);
  }
  pastSensorFlag = currentSensorFlag;


  //Serial.print(current);
  //Serial.print(' ');
  //Serial.println(sensorSum);
  
}
