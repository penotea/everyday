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
 * - 11: 普通のLED (音量に連動して明度変化)
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
#include "PhotoReflectorModule.h"

#define CONTROL_RATE 64 // Hz, powers of 2 are most reliable

// 普通のLED設定
#define LED_PIN 11

// フォトリフレクタ設定
const int sensorNum = 3;
PhotoReflectorModule sensors[sensorNum];

// G#メジャースケール（音程設定）
const int FREQ_GS4 = 415;  // G# (1度)
const int FREQ_AS4 = 466;  // A# (2度)
const int FREQ_C5 = 523;   // C  (3度)
const int FREQ_CS5 = 554;  // C# (4度)
const int FREQ_DS5 = 622;  // D# (5度)
const int FREQ_F5 = 698;   // F  (6度)
const int FREQ_G5 = 784;   // G  (7度)

// センサーごとの音程グループ
const int sensor1_notes[] = {FREQ_GS4, FREQ_C5, FREQ_DS5};    // 1,3,5度
const int sensor2_notes[] = {FREQ_AS4, FREQ_CS5, FREQ_F5};    // 2,4,6度
const int sensor3_notes[] = {FREQ_C5, FREQ_DS5, FREQ_G5};     // 3,5,7度

// 音声チャンネル構造体（音楽的拡張版）
struct VoiceChannel {
  Oscil<SQUARE_NO_ALIAS_2048_NUM_CELLS, AUDIO_RATE>* fundamental;  // 基本音オシレーター（矩形波）
  bool isActive;                                       // チャンネルがアクティブか
  unsigned long triggerTime;                           // 最初のトリガー時刻
  int currentRepeat;                                   // 現在の繰り返し回数
  int maxRepeats;                                      // このシーケンスの最大繰り返し回数（ランダム）
  unsigned long nextRepeatTime;                        // 次の発音時刻
  unsigned long noteEndTime;                           // 現在の音の終了時刻
  float currentVolume;                                 // 現在の音量倍率
  int sensorIndex;                                     // センサー番号（0-2）
  int currentFreq;                                     // 現在の周波数
};

// オシレーター（基本音用・矩形波）
Oscil<SQUARE_NO_ALIAS_2048_NUM_CELLS, AUDIO_RATE> oscC5(SQUARE_NO_ALIAS_2048_DATA);  // C5
Oscil<SQUARE_NO_ALIAS_2048_NUM_CELLS, AUDIO_RATE> oscE5(SQUARE_NO_ALIAS_2048_DATA);  // E5
Oscil<SQUARE_NO_ALIAS_2048_NUM_CELLS, AUDIO_RATE> oscG5(SQUARE_NO_ALIAS_2048_DATA);  // G5

// オシレーター（倍音用） - メモリ節約のため一時的にコメントアウト
// Oscil<SIN2048_NUM_CELLS, AUDIO_RATE> oscC6(SIN2048_DATA);  // C6
// Oscil<SIN2048_NUM_CELLS, AUDIO_RATE> oscE6(SIN2048_DATA);  // E6
// Oscil<SIN2048_NUM_CELLS, AUDIO_RATE> oscG6(SIN2048_DATA);  // G6

// 音の持続時間設定（ランダム化対応）
const int MIN_NOTE_DURATION_MS = 200;   // 最短音符（200ms）
const int MAX_NOTE_DURATION_MS = 2000;  // 最長音符（1000ms）

// 音声チャンネル配列
VoiceChannel voices[3];

// フィルター削除（シンプルな音声出力）

// 複数回発音設定（ランダム間隔対応）
const int MIN_INTERVAL_MS = 100;    // 最小間隔（100ms）
const int MAX_INTERVAL_MS = 2000;   // 最大間隔（2000ms）
const int MIN_REPEATS = 7;          // 最小ディレイ回数
const int MAX_REPEATS = 15;         // 最大ディレイ回数
const float VOLUME_DECAY = 0.5f;    // 音量減衰率（1/2）

// LED制御（音量連動対応）
int ledBrightness = 0;
bool ledActive = false;
const int MAX_LED_BRIGHTNESS = 255;

// 音量連動LED制御
int currentAudioLevel = 0;  // 現在の音量レベル（0-127）
int smoothedAudioLevel = 0; // スムージングされた音量レベル
const float AUDIO_SMOOTHING = 0.8f; // スムージング係数（0.8で滑らか）

// ランダム生成関数群
int getRandomInterval() {
  return random(MIN_INTERVAL_MS, MAX_INTERVAL_MS + 1);
}

int getRandomDuration() {
  return random(MIN_NOTE_DURATION_MS, MAX_NOTE_DURATION_MS + 1);
}

int getRandomNoteFromSensor(int sensorIndex) {
  switch(sensorIndex) {
    case 0: return sensor1_notes[random(3)];  // 1,3,5度からランダム選択
    case 1: return sensor2_notes[random(3)];  // 2,4,6度からランダム選択
    case 2: return sensor3_notes[random(3)];  // 3,5,7度からランダム選択
    default: return FREQ_C5;
  }
}

int getRandomRepeats() {
  return random(MIN_REPEATS, MAX_REPEATS + 1);
}

// ディレイ中の音程変更判定（30%の確率で変更）
bool shouldChangePitch() {
  return random(100) < 30;  // 30%の確率でtrue
}

// 音程変更または維持を決定
int getNextNoteFromSensor(int sensorIndex, int currentFreq) {
  if (shouldChangePitch()) {
    // 30%の確率で新しい音程
    return getRandomNoteFromSensor(sensorIndex);
  } else {
    // 70%の確率で現在の音程を維持
    return currentFreq;
  }
}

void setup() {
  // ランダムシード初期化
  randomSeed(analogRead(0));
  
  // Mozzi初期化
  startMozzi(CONTROL_RATE);
  
  // オシレーター設定（G#スケール対応）
  oscC5.setFreq(FREQ_C5);   // 初期設定（後で動的に変更）
  oscE5.setFreq(FREQ_DS5);  // 初期設定（後で動的に変更）
  oscG5.setFreq(FREQ_G5);   // 初期設定（後で動的に変更）
  
  // オシレーター設定（倍音） - メモリ節約のためコメントアウト
  // oscC6.setFreq(FREQ_C6);
  // oscE6.setFreq(FREQ_E6);
  // oscG6.setFreq(FREQ_G6);
  
  // 音声チャンネル初期化（音楽的拡張版）
  voices[0] = {&oscC5, false, 0, 0, 0, 0, 0, 1.0f, 0, FREQ_GS4};
  voices[1] = {&oscE5, false, 0, 0, 0, 0, 0, 1.0f, 1, FREQ_AS4};
  voices[2] = {&oscG5, false, 0, 0, 0, 0, 0, 1.0f, 2, FREQ_C5};
  
  // フィルター削除（シンプルな音声出力）
  
  // LED初期化
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  
  // フォトリフレクタセンサー初期化
  sensors[0].setup(12, A1);  // センサー0: LED=Pin12, Read=A1
  sensors[1].setup(10, A2);  // センサー1: LED=Pin10, Read=A2
  sensors[2].setup(8, A3);   // センサー2: LED=Pin8,  Read=A3
}

void updateControl() {
  unsigned long currentTime = millis();
  
  // LED制御（音量連動対応）
  // 音量レベルをスムージング
  smoothedAudioLevel = (int)(AUDIO_SMOOTHING * smoothedAudioLevel + (1.0f - AUDIO_SMOOTHING) * currentAudioLevel);
  
  // 音量に応じてLED明度を計算
  if (smoothedAudioLevel > 5) {  // 最小閾値（ノイズ除去）
    // 音量レベル（0-127）をLED明度（0-255）にマッピング
    ledBrightness = map(smoothedAudioLevel, 0, 127, 0, MAX_LED_BRIGHTNESS);
    ledBrightness = constrain(ledBrightness, 0, MAX_LED_BRIGHTNESS);
    analogWrite(LED_PIN, ledBrightness);
    ledActive = true;
  } else {
    // 音量が小さい場合は徐々にフェードアウト
    if (ledActive) {
      ledBrightness = ledBrightness * 0.95;  // ゆっくりフェードアウト
      if (ledBrightness < 5) {
        ledBrightness = 0;
        ledActive = false;
        digitalWrite(LED_PIN, LOW);
      } else {
        analogWrite(LED_PIN, ledBrightness);
      }
    }
  }
  
  // フォトリフレクタ更新
  sensorsUpdate();
  
  // 各センサーの検出処理（複数回発音対応）
  for (int i = 0; i < 3; i++) {
    if (sensors[i].isTriggered()) {
      // 新しい複数回発音シーケンス開始（完全ランダム化）
      voices[i].isActive = true;
      voices[i].triggerTime = currentTime;
      voices[i].currentRepeat = 0;
      voices[i].maxRepeats = getRandomRepeats();                     // ランダムディレイ回数
      voices[i].nextRepeatTime = currentTime + getRandomInterval();  // ランダム間隔
      voices[i].noteEndTime = currentTime + getRandomDuration();     // ランダム音長
      voices[i].currentVolume = 0.9f;
      voices[i].sensorIndex = i;
      
      // センサーに応じたランダム音程選択
      voices[i].currentFreq = getRandomNoteFromSensor(i);
      voices[i].fundamental->setFreq(voices[i].currentFreq);
    }
  }
  
  // 持続時間管理と複数回発音処理
  for (int i = 0; i < 3; i++) {
    if (voices[i].isActive) {
      // 現在の音が終了した場合の処理
      if (currentTime >= voices[i].noteEndTime) {
        // 次の発音タイミングをチェック
        if (voices[i].currentRepeat < voices[i].maxRepeats - 1 && 
            currentTime >= voices[i].nextRepeatTime) {
          
          // 次の発音を開始（完全ランダム化）
          voices[i].currentRepeat++;
          voices[i].currentVolume *= VOLUME_DECAY;  // 音量を1/2に減衰
          voices[i].nextRepeatTime = currentTime + getRandomInterval();  // 新ランダム間隔
          voices[i].noteEndTime = currentTime + getRandomDuration();     // 新ランダム音長
          
          // 音程変更判定（30%の確率で変更、70%で維持）
          voices[i].currentFreq = getNextNoteFromSensor(voices[i].sensorIndex, voices[i].currentFreq);
          voices[i].fundamental->setFreq(voices[i].currentFreq);
        } else if (voices[i].currentRepeat >= voices[i].maxRepeats - 1) {
          // 全ての発音が終了
          voices[i].isActive = false;
        }
      }
    }
  }
}

AudioOutput_t updateAudio() {
  int mixedSample = 0;
  
  // 全ての音声チャンネルをミキシング（持続時間ベース）
  unsigned long currentTime = millis();
  
  for (int i = 0; i < 3; i++) {
    if (voices[i].isActive && currentTime < voices[i].noteEndTime) {
      // 基本音の生成と音量適用
      int fundSample = voices[i].fundamental->next();
      // 現在の音量倍率を適用
      int fundOutput = (int)(fundSample * voices[i].currentVolume);
      mixedSample += fundOutput;
    }
  }
  
  // 音量ブーストなし（音割れ防止）
  // mixedSample = (mixedSample * 2) >> 1;  // ブースト削除
  
  // 基本的なクリッピング防止
  if (mixedSample > 127) mixedSample = 127;
  if (mixedSample < -128) mixedSample = -128;
  
  // 音量レベルを計算（LED制御用）
  currentAudioLevel = abs(mixedSample);  // 絶対値で音量レベルを取得（0-127）
  
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
