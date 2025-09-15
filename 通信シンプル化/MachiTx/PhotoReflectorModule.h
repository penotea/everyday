#include <Arduino.h>

class PhotoReflectorModule {
public:
  PhotoReflectorModule(){}

  void setup(int led, int read) {
    ledPin = led;
    readPin = read;

    pinMode(ledPin, OUTPUT);
    digitalWrite(ledPin, LOW);
    pinMode(readPin, INPUT);
  }

  void ledOn() {
    digitalWrite(ledPin, HIGH);
  }
  void ledOff() {
    digitalWrite(ledPin, LOW);
  }
  void readOff() {
    current = analogRead(readPin);  // オフのときのフォトトランジスタ値を取る
  }

  void readOn() {
    //フォトリフレクタプログラム
    delayMicroseconds(sensorLatency);
    current -= analogRead(readPin);    // オンのときで差分を取る（反射物があるときが正の値になる）
    digitalWrite(ledPin, LOW);  //IRLED消灯

    sensorSum += current;
    sensorSum -= history[historyIndex];
    history[historyIndex] = current;
    historyIndex++;
    if (historyIndex >= historyLength) historyIndex = 0;

    bool currentSensorFlag = sensorSum >= threshold;
    if (currentSensorFlag && !pastSensorFlag) {
      triggered = true;
    } else {
      triggered = false;
    }
    pastSensorFlag = currentSensorFlag;
  }

  bool isTriggered() {
    return triggered;
  }


private:
  int ledPin, readPin;

  static const uint16_t historyLength = 30;  //ここでオン・オフともに差分をためる分の長さ
  int32_t history[historyLength] = { 0 };
  uint32_t historyIndex = 0;
  int32_t sensorSum = 0;  //センサーオフのときの足し算したやつ
  int32_t current = 0;
  static const int32_t thresholdOnce = 4;  //ここで感度変えられそう(-分の差分) default:6
  static const int32_t threshold = thresholdOnce * historyLength;
  bool pastSensorFlag = false;
  bool triggered = false;

public:
  static const uint32_t sensorLatency = 20;  // センサーの応答特性（micro sec）default:30
};
