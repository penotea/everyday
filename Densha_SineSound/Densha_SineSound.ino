/*
 * Densha_SineSound - Photo Reflector Musical Instrument (Enhanced)
 * 
 * 機能:
 * - 3つのフォトリフレクタによる物体検出
 * - ポリフォニック音声（複数センサー同時発音対応）
 * - 倍音機能（1オクターブ上の遅延アタック音）
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
 * - センサー0 (左): C5 (523Hz) + C6 (1046Hz) - ド + 倍音
 * - センサー1 (中央): E5 (659Hz) + E6 (1318Hz) - ミ + 倍音
 * - センサー2 (右): G5 (784Hz) + G6 (1568Hz) - ソ + 倍音
 */

#include <MozziGuts.h>
#include <Oscil.h>
#include <tables/sin2048_int8.h>
#include <tables/square_no_alias_2048_int8.h>
#include <ADSR.h>
#include <LowPassFilter.h>
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

// 音程設定（基本音と倍音）
const int FREQ_C5 = 523;  // ド
const int FREQ_E5 = 659;  // ミ
const int FREQ_G5 = 784;  // ソ
const int FREQ_C6 = 1046; // ド（1オクターブ上）
const int FREQ_E6 = 1318; // ミ（1オクターブ上）
const int FREQ_G6 = 1568; // ソ（1オクターブ上）

// 音声チャンネル構造体（複数回発音対応）
struct VoiceChannel {
  Oscil<SQUARE_NO_ALIAS_2048_NUM_CELLS, AUDIO_RATE>* fundamental;  // 基本音オシレーター（矩形波）
  ADSR<CONTROL_RATE, AUDIO_RATE>* fundEnvelope;       // 基本音エンベロープ
  bool isActive;                                       // チャンネルがアクティブか
  unsigned long triggerTime;                           // 最初のトリガー時刻
  int currentRepeat;                                   // 現在の繰り返し回数（0-3）
  unsigned long nextRepeatTime;                        // 次の発音時刻
  float currentVolume;                                 // 現在の音量倍率
};

// オシレーター（基本音用・矩形波）
Oscil<SQUARE_NO_ALIAS_2048_NUM_CELLS, AUDIO_RATE> oscC5(SQUARE_NO_ALIAS_2048_DATA);  // C5
Oscil<SQUARE_NO_ALIAS_2048_NUM_CELLS, AUDIO_RATE> oscE5(SQUARE_NO_ALIAS_2048_DATA);  // E5
Oscil<SQUARE_NO_ALIAS_2048_NUM_CELLS, AUDIO_RATE> oscG5(SQUARE_NO_ALIAS_2048_DATA);  // G5

// オシレーター（倍音用） - メモリ節約のため一時的にコメントアウト
// Oscil<SIN2048_NUM_CELLS, AUDIO_RATE> oscC6(SIN2048_DATA);  // C6
// Oscil<SIN2048_NUM_CELLS, AUDIO_RATE> oscE6(SIN2048_DATA);  // E6
// Oscil<SIN2048_NUM_CELLS, AUDIO_RATE> oscG6(SIN2048_DATA);  // G6

// エンベロープ（基本音用）
ADSR<CONTROL_RATE, AUDIO_RATE> fundEnv0;
ADSR<CONTROL_RATE, AUDIO_RATE> fundEnv1;
ADSR<CONTROL_RATE, AUDIO_RATE> fundEnv2;

// エンベロープ（倍音用） - メモリ節約のため一時的にコメントアウト
// ADSR<CONTROL_RATE, AUDIO_RATE> harmEnv0;
// ADSR<CONTROL_RATE, AUDIO_RATE> harmEnv1;
// ADSR<CONTROL_RATE, AUDIO_RATE> harmEnv2;

// 音声チャンネル配列
VoiceChannel voices[3];

// ローパスフィルター（ハイカット用）
LowPassFilter lpf;

// DCブロッキングフィルター（ハイパス効果・音割れ防止）
int prevSample = 0;
int prevOutput = 0;
const float DC_BLOCK_COEFF = 0.95f;  // DCブロッキング係数

// 複数回発音設定（ランダム間隔対応）
const int MIN_INTERVAL_MS = 100;    // 最小間隔（100ms）
const int MAX_INTERVAL_MS = 500;    // 最大間隔（500ms）
const int MAX_REPEATS = 10;         // 最大10回発音
const float VOLUME_DECAY = 0.5f;    // 音量減衰率（1/2）

// LED制御
int brightness = 0;
int ledflag = 0;
unsigned long prevtime = 0;
const unsigned long intervaltime = 1;

// ランダム間隔生成関数
int getRandomInterval() {
  return random(MIN_INTERVAL_MS, MAX_INTERVAL_MS + 1);
}

void setup() {
  // ランダムシード初期化
  randomSeed(analogRead(0));
  
  // Mozzi初期化
  startMozzi(CONTROL_RATE);
  
  // オシレーター設定（基本音）
  oscC5.setFreq(FREQ_C5);
  oscE5.setFreq(FREQ_E5);
  oscG5.setFreq(FREQ_G5);
  
  // オシレーター設定（倍音） - メモリ節約のためコメントアウト
  // oscC6.setFreq(FREQ_C6);
  // oscE6.setFreq(FREQ_E6);
  // oscG6.setFreq(FREQ_G6);
  
  // 基本音エンベロープ設定（即座にアタック）
  fundEnv0.setADLevels(255, 64);
  fundEnv0.setTimes(0, 0, 0, 400);
  fundEnv1.setADLevels(255, 64);
  fundEnv1.setTimes(0, 0, 0, 400);
  fundEnv2.setADLevels(255, 64);
  fundEnv2.setTimes(0, 0, 0, 400);
  
  // 倍音エンベロープ設定 - メモリ節約のためコメントアウト
  // harmEnv0.setADLevels(128, 32);
  // harmEnv0.setTimes(2000, 0, 0, 3500);
  // harmEnv1.setADLevels(128, 32);
  // harmEnv1.setTimes(2000, 0, 0, 3500);
  // harmEnv2.setADLevels(128, 32);
  // harmEnv2.setTimes(2000, 0, 0, 3500);
  
  // 音声チャンネル初期化（複数回発音対応）
  voices[0] = {&oscC5, &fundEnv0, false, 0, 0, 0, 0.5f};
  voices[1] = {&oscE5, &fundEnv1, false, 0, 0, 0, 0.5f};
  voices[2] = {&oscG5, &fundEnv2, false, 0, 0, 0, 0.5f};
  
  // ローパスフィルター設定（3000Hz以下を通す）
  lpf.setCutoffFreq(200);  // カットオフ周波数を設定（約3000Hz相当）
  
  // DCブロッキングフィルター初期化
  prevSample = 0;
  prevOutput = 0;
  
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
  
  // 各センサーの検出処理（複数回発音対応）
  for (int i = 0; i < 3; i++) {
    if (sensors[i].isTriggered()) {
      // 新しい複数回発音シーケンス開始
      voices[i].isActive = true;
      voices[i].triggerTime = currentTime;
      voices[i].currentRepeat = 0;
      voices[i].nextRepeatTime = currentTime + getRandomInterval();  // 次回はランダム間隔後
      voices[i].currentVolume = 1.0f;
      voices[i].fundEnvelope->noteOn();  // 1回目の発音開始
      ledflag = 1;  // LED点滅開始
    }
  }
  
  // 全エンベロープ更新と複数回発音処理
  for (int i = 0; i < 3; i++) {
    if (voices[i].isActive) {
      voices[i].fundEnvelope->update();
      
      // エンベロープが終了した場合の処理
      if (voices[i].fundEnvelope->playing() == false) {
        // 次の発音タイミングをチェック
        if (voices[i].currentRepeat < MAX_REPEATS - 1 && 
            currentTime >= voices[i].nextRepeatTime) {
          
          // 次の発音を開始
          voices[i].currentRepeat++;
          voices[i].currentVolume *= VOLUME_DECAY;  // 音量を1/2に減衰
          voices[i].nextRepeatTime = currentTime + getRandomInterval();  // 次回もランダム間隔
          voices[i].fundEnvelope->noteOn();  // 次の発音開始
        } else if (voices[i].currentRepeat >= MAX_REPEATS - 1) {
          // 全ての発音が終了
          voices[i].isActive = false;
        }
      }
    }
  }
}

AudioOutput_t updateAudio() {
  int mixedSample = 0;
  
  // 全ての音声チャンネルをミキシング（音量減衰対応）
  for (int i = 0; i < 3; i++) {
    if (voices[i].isActive) {
      // 基本音の生成とエンベロープ適用
      int fundSample = voices[i].fundamental->next();
      int fundEnvValue = voices[i].fundEnvelope->next();
      if (fundEnvValue > 0) {
        // 現在の音量倍率を適用
        int fundOutput = (fundSample * fundEnvValue) >> 8;
        fundOutput = (int)(fundOutput * voices[i].currentVolume);
        mixedSample += fundOutput;
      }
    }
  }
  
  // DCブロッキングフィルター適用（ハイパス効果・音割れ防止）
  // y[n] = x[n] - x[n-1] + α * y[n-1] (DCブロッキング)
  int currentOutput = mixedSample - prevSample + (int)(DC_BLOCK_COEFF * prevOutput);
  prevSample = mixedSample;
  prevOutput = currentOutput;
  mixedSample = currentOutput;
  
  // ローパスフィルター適用（ハイカット）
  mixedSample = lpf.next(mixedSample);
  
  // フィルター補償による軽い音量ブースト（音割れしない範囲で）
  mixedSample = (mixedSample * 3) >> 1;  // 1.5倍程度
  
  // ソフトクリッピング（音割れを滑らかに制限）
  if (mixedSample > 100) mixedSample = 100 + (mixedSample - 100) / 4;
  if (mixedSample < -100) mixedSample = -100 + (mixedSample + 100) / 4;
  
  // 最終クリッピング防止
  if (mixedSample > 127) mixedSample = 127;
  if (mixedSample < -128) mixedSample = -128;
  
  return MonoOutput::from8Bit(mixedSample);
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
