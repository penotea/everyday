#include <Adafruit_NeoPixel.h>
#include "PhotoReflectorModule.h"

// NeoPixel設定
#define PIN 11  // NeoPixel pin
#define NUMPIXELS 1
Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

// 音声出力ピン（PB1 = Pin 9）
#define AUDIO_PIN 9

// フォトリフレクター設定
const int sensorNum = 3;
PhotoReflectorModule sensors[sensorNum];

// 音程の周波数 (Hz)
const float frequencies[] = {
  146.83,  // D3 (センサーA)
  174.61,  // F3 (センサーB)
  220.00   // A3 (センサーC)
};

// 倍音の周波数 (1オクターブ上)
const float harmonicFrequencies[] = {
  293.66,  // D4 (センサーA倍音)
  349.23,  // F4 (センサーB倍音)
  440.00   // A4 (センサーC倍音)
};

// LED色設定 (RGB)
const uint32_t colors[] = {
  pixels.Color(0, 0, 255),     // 青 (センサーA)
  pixels.Color(128, 0, 128),   // 紫 (センサーB)
  pixels.Color(0, 255, 128)    // 青緑 (センサーC)
};

// フェードアウト管理用構造体（拡張版）
struct FadeState {
  bool active;
  unsigned long startTime;
  int sensorIndex;
  float currentAmplitude;
  uint8_t currentBrightness;
  
  // 倍音用追加パラメータ
  bool harmonicActive;
  float harmonicAmplitude;
  unsigned long harmonicStartTime;
};

FadeState fadeStates[3];
const unsigned long FADE_DURATION = 3000; // 3秒間のフェードアウト
const unsigned long HARMONIC_ATTACK_TIME = 500; // 倍音のアタック時間（0.5秒）

// サイン波テーブル (256サンプル)
const int SINE_TABLE_SIZE = 256;
int sineTable[SINE_TABLE_SIZE];

// サイン波生成用変数
float phaseAccumulator[3] = {0, 0, 0};
float harmonicPhaseAccumulator[3] = {0, 0, 0}; // 倍音用位相累積器
float phaseIncrement[3];
float harmonicPhaseIncrement[3]; // 倍音用位相増分
const float SAMPLE_RATE = 8000.0; // 8kHz sampling rate

void setup() {
  Serial.begin(9600);
  
  // NeoPixel初期化
  pixels.begin();
  pixels.clear();
  pixels.show();
  
  // 音声出力ピン設定
  pinMode(AUDIO_PIN, OUTPUT);
  
  // サイン波テーブル生成
  generateSineTable();
  
  // 位相増分計算
  for (int i = 0; i < 3; i++) {
    phaseIncrement[i] = (frequencies[i] * SINE_TABLE_SIZE) / SAMPLE_RATE;
    harmonicPhaseIncrement[i] = (harmonicFrequencies[i] * SINE_TABLE_SIZE) / SAMPLE_RATE;
  }
  
  // フォトリフレクター初期化
  sensors[0].setup(12, A1); // センサーA
  sensors[1].setup(10, A2); // センサーB
  sensors[2].setup(8, A3);  // センサーC
  
  // フェード状態初期化
  for (int i = 0; i < 3; i++) {
    fadeStates[i].active = false;
    fadeStates[i].currentAmplitude = 0;
    fadeStates[i].currentBrightness = 0;
    fadeStates[i].harmonicActive = false;
    fadeStates[i].harmonicAmplitude = 0;
  }
  
  // タイマー1設定（音声生成用）
  setupTimer1();
  
  Serial.println("Enhanced PhotoReflector Sound & Light System Ready");
  Serial.println("Features: Polyphonic + Harmonic Layer");
}

void loop() {
  sensorsUpdate();
  
  // 各センサーの状態チェック
  for (int i = 0; i < 3; i++) {
    if (sensors[i].isTriggered()) {
      // 改善点1: 同じセンサーでも重複トリガー可能にする
      triggerSoundAndLight(i);
    }
  }
  
  // フェードアウト処理
  updateFadeStates();
  
  // LED更新
  updateLEDs();
  
  delay(1); // 短い待機時間
}

void triggerSoundAndLight(int sensorIndex) {
  Serial.print("Sensor ");
  Serial.print((char)('A' + sensorIndex));
  Serial.println(" triggered!");
  
  // 基本音のフェード状態開始（重複トリガー対応）
  fadeStates[sensorIndex].active = true;
  fadeStates[sensorIndex].startTime = millis();
  fadeStates[sensorIndex].sensorIndex = sensorIndex;
  fadeStates[sensorIndex].currentAmplitude = 1.0;
  fadeStates[sensorIndex].currentBrightness = 255;
  
  // 倍音のフェード状態開始
  fadeStates[sensorIndex].harmonicActive = true;
  fadeStates[sensorIndex].harmonicStartTime = millis();
  fadeStates[sensorIndex].harmonicAmplitude = 0; // アタックは0から開始
  
  // 位相リセット
  phaseAccumulator[sensorIndex] = 0;
  harmonicPhaseAccumulator[sensorIndex] = 0;
}

void updateFadeStates() {
  unsigned long currentTime = millis();
  
  for (int i = 0; i < 3; i++) {
    // 基本音の処理
    if (fadeStates[i].active) {
      unsigned long elapsed = currentTime - fadeStates[i].startTime;
      
      if (elapsed >= FADE_DURATION) {
        // フェードアウト完了
        fadeStates[i].active = false;
        fadeStates[i].currentAmplitude = 0;
        fadeStates[i].currentBrightness = 0;
      } else {
        // フェードアウト中
        float progress = (float)elapsed / FADE_DURATION;
        float fadeValue = 1.0 - progress;
        
        // 指数関数的な減衰カーブ
        fadeValue = fadeValue * fadeValue;
        
        fadeStates[i].currentAmplitude = fadeValue;
        fadeStates[i].currentBrightness = (uint8_t)(255 * fadeValue);
      }
    }
    
    // 倍音の処理
    if (fadeStates[i].harmonicActive) {
      unsigned long harmonicElapsed = currentTime - fadeStates[i].harmonicStartTime;
      
      if (harmonicElapsed >= FADE_DURATION) {
        // 倍音フェードアウト完了
        fadeStates[i].harmonicActive = false;
        fadeStates[i].harmonicAmplitude = 0;
      } else {
        float totalProgress = (float)harmonicElapsed / FADE_DURATION;
        
        if (harmonicElapsed < HARMONIC_ATTACK_TIME) {
          // アタック期間中（0から0.5の音量まで上昇）
          float attackProgress = (float)harmonicElapsed / HARMONIC_ATTACK_TIME;
          fadeStates[i].harmonicAmplitude = 0.5 * attackProgress; // 最大音量は半分
        } else {
          // フェードアウト期間
          float fadeProgress = (float)(harmonicElapsed - HARMONIC_ATTACK_TIME) / (FADE_DURATION - HARMONIC_ATTACK_TIME);
          float fadeValue = 1.0 - fadeProgress;
          fadeValue = fadeValue * fadeValue; // 指数関数的減衰
          fadeStates[i].harmonicAmplitude = 0.5 * fadeValue;
        }
      }
    }
  }
}

void updateLEDs() {
  // 最も明るいセンサーの色を表示
  int brightestSensor = -1;
  uint8_t maxBrightness = 0;
  
  for (int i = 0; i < 3; i++) {
    if (fadeStates[i].active && fadeStates[i].currentBrightness > maxBrightness) {
      maxBrightness = fadeStates[i].currentBrightness;
      brightestSensor = i;
    }
  }
  
  if (brightestSensor >= 0) {
    // 色を明度で調整
    uint32_t color = colors[brightestSensor];
    uint8_t r = (uint8_t)((color >> 16) & 0xFF);
    uint8_t g = (uint8_t)((color >> 8) & 0xFF);
    uint8_t b = (uint8_t)(color & 0xFF);
    
    r = (r * maxBrightness) / 255;
    g = (g * maxBrightness) / 255;
    b = (b * maxBrightness) / 255;
    
    pixels.setPixelColor(0, pixels.Color(r, g, b));
  } else {
    pixels.clear();
  }
  
  pixels.show();
}

void generateSineTable() {
  for (int i = 0; i < SINE_TABLE_SIZE; i++) {
    float angle = (2.0 * PI * i) / SINE_TABLE_SIZE;
    sineTable[i] = (int)(127 * sin(angle));
  }
}

void setupTimer1() {
  // Timer1をCTCモードで設定
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1 = 0;
  
  // 8kHzのサンプリング周波数設定
  OCR1A = 1999; // 16MHz / (1 * (1999+1)) = 8kHz
  TCCR1B |= (1 << WGM12); // CTCモード
  TCCR1B |= (1 << CS10);  // プリスケーラ1
  TIMSK1 |= (1 << OCIE1A); // Compare Aマッチ割り込み有効
}

// タイマー1割り込みハンドラ（音声生成）- 拡張版
ISR(TIMER1_COMPA_vect) {
  int mixedSample = 0;
  
  // 全ての有効なセンサーの基本音と倍音を混合
  for (int i = 0; i < 3; i++) {
    // 基本音の処理
    if (fadeStates[i].active && fadeStates[i].currentAmplitude > 0) {
      // サイン波サンプル取得
      int tableIndex = (int)phaseAccumulator[i] % SINE_TABLE_SIZE;
      int sample = sineTable[tableIndex];
      
      // 振幅適用
      sample = (int)(sample * fadeStates[i].currentAmplitude);
      mixedSample += sample;
      
      // 位相更新
      phaseAccumulator[i] += phaseIncrement[i];
      if (phaseAccumulator[i] >= SINE_TABLE_SIZE) {
        phaseAccumulator[i] -= SINE_TABLE_SIZE;
      }
    }
    
    // 倍音の処理
    if (fadeStates[i].harmonicActive && fadeStates[i].harmonicAmplitude > 0) {
      // 倍音のサイン波サンプル取得
      int harmonicTableIndex = (int)harmonicPhaseAccumulator[i] % SINE_TABLE_SIZE;
      int harmonicSample = sineTable[harmonicTableIndex];
      
      // 倍音の振幅適用（基本音の半分の音量）
      harmonicSample = (int)(harmonicSample * fadeStates[i].harmonicAmplitude);
      mixedSample += harmonicSample;
      
      // 倍音の位相更新
      harmonicPhaseAccumulator[i] += harmonicPhaseIncrement[i];
      if (harmonicPhaseAccumulator[i] >= SINE_TABLE_SIZE) {
        harmonicPhaseAccumulator[i] -= SINE_TABLE_SIZE;
      }
    }
  }
  
  // 出力範囲調整とPWM出力
  mixedSample = constrain(mixedSample + 127, 0, 255);
  analogWrite(AUDIO_PIN, mixedSample);
}

// フォトリフレクターセンシング（既存コードから流用）
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
