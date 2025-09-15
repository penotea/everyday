/*
 * Mozzi Bell Generator - Sine/Square Wave with Envelope
 * 
 * 機能:
 * - Mozziライブラリによる高品質音声生成
 * - サイン波/スクエア波の選択
 * - エンベロープ付き鐘音
 * - 自動周期音（3秒間隔）
 * 
 * ピン配置:
 * - 9: 音声出力 (Mozzi PWM)
 * - 2: DIPスイッチ1 (HIGH=Sine波, LOW=Triangle波+フィルター)
 * 
 * 動作: 一定周期ごとに自動で鐘音を鳴らします
 */

#include <MozziGuts.h>
#include <Oscil.h>
#include <tables/sin2048_int8.h>
#include <tables/triangle2048_int8.h>
#include <ADSR.h>
#include <LowPassFilter.h>

#define CONTROL_RATE 64 // Hz, powers of 2 are most reliable
#define DIP_SWITCH_1_PIN 2  // 波形選択(sine/square)

// オシレーター
Oscil<SIN2048_NUM_CELLS, AUDIO_RATE> aSin(SIN2048_DATA);
Oscil<TRIANGLE2048_NUM_CELLS, AUDIO_RATE> aTriangle(TRIANGLE2048_DATA);

// エンベロープ
ADSR <CONTROL_RATE, AUDIO_RATE> envelope;

// ローパスフィルター（スクエア波を滑らかにするため）
LowPassFilter lpf;

// 設定
bool useSineWave = true;
unsigned long lastBellTime = 0;
const unsigned long BELL_INTERVAL = 3000; // 3秒間隔
const int FREQUENCY = 880; // A5音

void setup() {
  // ピン設定
  pinMode(DIP_SWITCH_1_PIN, INPUT_PULLUP);
  
  // Mozzi初期化
  startMozzi(CONTROL_RATE);
  
  // オシレーター設定
  aSin.setFreq(FREQUENCY);
  aTriangle.setFreq(FREQUENCY);
  
  // ローパスフィルター設定（カットオフ周波数）
  lpf.setCutoffFreq(100); // 200Hzでカットオフ（丸い音色に）
  
  // エンベロープ設定（鐘のような音）
  envelope.setADLevels(255, 64);  // アタック、ディケイレベル
  envelope.setTimes(0, 0, 0, 1500); // アタック、ディケイ、サステイン、リリース時間
  
  // 最初の音をトリガー
  lastBellTime = millis();
  envelope.noteOn();
}

void updateControl() {
  // DIPスイッチの状態を確認
  useSineWave = (digitalRead(DIP_SWITCH_1_PIN) == HIGH);
  
  // 一定周期ごとに音を鳴らす
  unsigned long currentTime = millis();
  if (currentTime - lastBellTime >= BELL_INTERVAL) {
    envelope.noteOn();
    lastBellTime = currentTime;
  }
  
  // エンベロープ更新
  envelope.update();
}

AudioOutput_t updateAudio() {
  int sample;
  
  if (useSineWave) {
    // サイン波生成（音量を2倍にブースト）
    sample = (aSin.next() * 8) >> 1; // 音量アップ
  } else {
    // 三角波生成（丸いスクエア波の代替）
    sample = aTriangle.next();
    
    // さらに丸くするためにローパスフィルターを適用
    sample = lpf.next(sample);
    
    // 音量を少しブーストして音量バランスを取る
    sample = (sample * 3) >> 1;
  }
  
  // エンベロープ適用
  sample = (sample * envelope.next()) >> 8;
  
  return MonoOutput::from8Bit(sample);
}

void loop() {
  audioHook(); // Mozziのメインループ
}
