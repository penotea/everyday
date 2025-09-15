/*
 * Densha_SineSound - Photo Reflector Musical Instrument
 * 
 * 機能:
 * - 3つのフォトリフレクタによる物体検出
 * - 位置に応じた音程のサイン波生成
 * - Mozziライブラリによる高品質音声
 * - 鐘のようなエンベロープ
 * 
 * ピン配置:
 * - 9: 音声出力 (Mozzi PWM)
 * - 8, 10, 12: フォトリフレクタLED制御
 * - A1, A2, A3: フォトリフレクタ読み取り
 * - 11: NeoPixel LED (オプション)
 * 
 * 音程マッピング:
 * - センサー0 (左): C5 (523Hz) - ド
 * - センサー1 (中央): E5 (659Hz) - ミ
 * - センサー2 (右): G5 (784Hz) - ソ
 */

#include <MozziGuts.h>
#include <Oscil.h>
#include <tables/sin2048_int8.h>
#include <ADSR.h>
#include <Adafruit_NeoPixel.h>
#include "PhotoReflectorModule.h"

#define CONTROL_RATE 64 // Hz, powers of 2 are most reliable

// NeoPixel設定
#define NEOPIXEL_PIN 11
#define NUMPIXELS 1
Adafruit_NeoPixel pixels(NUMPIXELS, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

// フォトリフレクタ設定
const int sensorNum = 3;
PhotoReflectorModule sensors[sensorNum];

// オシレーター（3つの音程用）
Oscil<SIN2048_NUM_CELLS, AUDIO_RATE> oscC5(SIN2048_DATA);  // C5 (523Hz)
Oscil<SIN2048_NUM_CELLS, AUDIO_RATE> oscE5(SIN2048_DATA);  // E5 (659Hz)
Oscil<SIN2048_NUM_CELLS, AUDIO_RATE> oscG5(SIN2048_DATA);  // G5 (784Hz)

// エンベロープ
ADSR <CONTROL_RATE, AUDIO_RATE> envelope;

// 音程設定
const int FREQ_C5 = 523;  // ド
const int FREQ_E5 = 659;  // ミ
const int FREQ_G5 = 784;  // ソ

// 現在の音程と状態
int currentFreq = 0;
bool isPlaying = false;

// LED制御
int brightness = 0;
int ledflag = 0;
unsigned long prevtime = 0;
const unsigned long intervaltime = 1;

void setup() {
  // Mozzi初期化
  startMozzi(CONTROL_RATE);
  
  // オシレーター設定
  oscC5.setFreq(FREQ_C5);
  oscE5.setFreq(FREQ_E5);
  oscG5.setFreq(FREQ_G5);
  
  // エンベロープ設定（鐘のような音）
  envelope.setADLevels(255, 64);  // アタック、ディケイレベル
  envelope.setTimes(0, 0, 0, 2000); // アタック、ディケイ、サステイン、リリース時間
  
  // NeoPixel初期化
  pixels.begin();
  pixels.setPixelColor(0, pixels.Color(100, 100, 100));
  pixels.show();
  delay(1000);
  pixels.clear();
  
  // フォトリフレクタセンサー初期化
  sensors[0].setup(12, A1);  // センサー0: LED=Pin12, Read=A1
  sensors[1].setup(10, A2);  // センサー1: LED=Pin10, Read=A2
  sensors[2].setup(8, A3);   // センサー2: LED=Pin8,  Read=A3
}

void updateControl() {
  unsigned long currentTime = millis();
  
  // NeoPixel LED制御
  if ((currentTime - prevtime) >= intervaltime) {
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
  
  // NeoPixel表示
  pixels.setPixelColor(0, pixels.Color(0, 0, brightness));
  pixels.show();
  if (brightness == 0) pixels.clear();
  
  // フォトリフレクタ更新
  sensorsUpdate();
  
  // センサー0検出 (C5 - ド)
  if (sensors[0].isTriggered()) {
    currentFreq = FREQ_C5;
    envelope.noteOn();
    isPlaying = true;
    ledflag = 1;  // LED点滅開始
  }
  
  // センサー1検出 (E5 - ミ)
  if (sensors[1].isTriggered()) {
    currentFreq = FREQ_E5;
    envelope.noteOn();
    isPlaying = true;
    ledflag = 1;  // LED点滅開始
  }
  
  // センサー2検出 (G5 - ソ)
  if (sensors[2].isTriggered()) {
    currentFreq = FREQ_G5;
    envelope.noteOn();
    isPlaying = true;
    ledflag = 1;  // LED点滅開始
  }
  
  // エンベロープ更新
  envelope.update();
}

AudioOutput_t updateAudio() {
  int sample = 0;
  
  if (isPlaying && envelope.next() > 0) {
    // 現在の音程に応じてサイン波生成
    if (currentFreq == FREQ_C5) {
      sample = oscC5.next();
    } else if (currentFreq == FREQ_E5) {
      sample = oscE5.next();
    } else if (currentFreq == FREQ_G5) {
      sample = oscG5.next();
    }
    
    // 音量ブースト
    sample = (sample * 8) >> 1;
    
    // エンベロープ適用
    sample = (sample * envelope.next()) >> 8;
  } else {
    // 音が終わったら停止
    isPlaying = false;
  }
  
  return MonoOutput::from8Bit(sample);
}

void loop() {
  audioHook(); // Mozziのメインループ
}


// フォトリフレクターセンシング（参考プログラムから）
void sensorsUpdate() {
  // 全センサーのOFF値を読み取り
  for (int i = 0; i < sensorNum; ++i) {
    sensors[i].readOff();
  }
  
  // 全センサーのLEDをON
  for (int i = 0; i < sensorNum; ++i) {
    sensors[i].ledOn();
  }
  
  // センサー応答待ち
  delayMicroseconds(PhotoReflectorModule::sensorLatency);
  
  // 全センサーのON値を読み取り
  for (int i = 0; i < sensorNum; ++i) {
    sensors[i].readOn();
  }
  
  // 全センサーのLEDをOFF
  for (int i = 0; i < sensorNum; ++i) {
    sensors[i].ledOff();
  }
}
