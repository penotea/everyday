/*
 * Sinetest - サイン波/矩形波テストプログラム
 * 
 * 機能:
 * - 9ピンから一定周期でサイン波を出力
 * - アタック200ms、リリース1秒のエンベロープ
 * - DIPスイッチで矩形波に切り替え可能
 * 
 * ピン配置:
 * - 9: 音声出力 (PWM)
 * - 2: DIPスイッチ (LOW=サイン波, HIGH=矩形波)
 */

// 音声出力ピン
#define AUDIO_PIN 9

// DIPスイッチピン
#define DIP_SWITCH_PIN 2

// 音程設定 (Hz)
const float FREQUENCY = 440.0; // A4音

// エンベロープ設定
const unsigned long ATTACK_TIME = 200;   // アタック時間 200ms
const unsigned long RELEASE_TIME = 1000; // リリース時間 1秒
const unsigned long CYCLE_INTERVAL = 3000; // 3秒ごとに再生

// サイン波テーブル設定
const int SINE_TABLE_SIZE = 256;
int sineTable[SINE_TABLE_SIZE];

// 音声生成用変数
float phaseAccumulator = 0;
float phaseIncrement;
const float SAMPLE_RATE = 8000.0; // 8kHzサンプリング

// エンベロープ管理用変数
struct EnvelopeState {
  bool active;
  unsigned long startTime;
  float amplitude;
  enum Phase { ATTACK, RELEASE, IDLE } phase;
} envelope;

// 波形選択
bool useSquareWave = false;

// タイミング管理
unsigned long lastTriggerTime = 0;

void setup() {
  Serial.begin(9600);
  
  // ピン設定
  pinMode(AUDIO_PIN, OUTPUT);
  pinMode(DIP_SWITCH_PIN, INPUT_PULLUP);
  
  // サイン波テーブル生成
  generateSineTable();
  
  // 位相増分計算
  phaseIncrement = (FREQUENCY * SINE_TABLE_SIZE) / SAMPLE_RATE;
  
  // エンベロープ初期化
  envelope.active = false;
  envelope.amplitude = 0;
  envelope.phase = EnvelopeState::IDLE;
  
  // タイマー1設定（音声生成用）
  setupTimer1();
  
  Serial.println("Sinetest Ready");
  Serial.println("DIP Switch: LOW=Sine Wave, HIGH=Square Wave");
}

void loop() {
  // DIPスイッチ状態読み取り
  useSquareWave = (digitalRead(DIP_SWITCH_PIN) == HIGH);
  
  // 一定周期でサウンド再生
  unsigned long currentTime = millis();
  if (currentTime - lastTriggerTime >= CYCLE_INTERVAL) {
    triggerSound();
    lastTriggerTime = currentTime;
  }
  
  // エンベロープ更新
  updateEnvelope();
  
  delay(1);
}

void triggerSound() {
  Serial.print("Playing ");
  Serial.print(useSquareWave ? "Square" : "Sine");
  Serial.println(" wave");
  
  // エンベロープ開始
  envelope.active = true;
  envelope.startTime = millis();
  envelope.phase = EnvelopeState::ATTACK;
  envelope.amplitude = 0;
  
  // 位相リセット
  phaseAccumulator = 0;
}

void updateEnvelope() {
  if (!envelope.active) return;
  
  unsigned long currentTime = millis();
  unsigned long elapsed = currentTime - envelope.startTime;
  
  switch (envelope.phase) {
    case EnvelopeState::ATTACK:
      if (elapsed < ATTACK_TIME) {
        // アタック中: 0から1まで線形増加
        envelope.amplitude = (float)elapsed / ATTACK_TIME;
      } else {
        // アタック完了、リリース開始
        envelope.phase = EnvelopeState::RELEASE;
        envelope.startTime = currentTime;
      }
      break;
      
    case EnvelopeState::RELEASE:
      if (elapsed < RELEASE_TIME) {
        // リリース中: 1から0まで指数関数的減衰
        float progress = (float)elapsed / RELEASE_TIME;
        envelope.amplitude = (1.0 - progress) * (1.0 - progress);
      } else {
        // リリース完了
        envelope.active = false;
        envelope.amplitude = 0;
        envelope.phase = EnvelopeState::IDLE;
      }
      break;
      
    case EnvelopeState::IDLE:
      envelope.amplitude = 0;
      break;
  }
}

void generateSineTable() {
  for (int i = 0; i < SINE_TABLE_SIZE; i++) {
    float angle = (2.0 * PI * i) / SINE_TABLE_SIZE;
    // tone()と同じ音量レベルに調整 (約50%の振幅)
    sineTable[i] = (int)(64 * sin(angle)); // 127 -> 64に変更
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

// タイマー1割り込みハンドラ（音声生成）
ISR(TIMER1_COMPA_vect) {
  int sample = 0;
  
  if (envelope.active && envelope.amplitude > 0) {
    if (useSquareWave) {
      // 矩形波生成 - tone()と同じ音量レベル
      int tableIndex = (int)phaseAccumulator % SINE_TABLE_SIZE;
      sample = (tableIndex < SINE_TABLE_SIZE / 2) ? 64 : -64; // 127 -> 64に変更
    } else {
      // サイン波生成
      int tableIndex = (int)phaseAccumulator % SINE_TABLE_SIZE;
      sample = sineTable[tableIndex];
    }
    
    // エンベロープ適用
    sample = (int)(sample * envelope.amplitude);
    
    // 位相更新
    phaseAccumulator += phaseIncrement;
    if (phaseAccumulator >= SINE_TABLE_SIZE) {
      phaseAccumulator -= SINE_TABLE_SIZE;
    }
  }
  
  // 出力範囲調整とPWM出力
  sample = constrain(sample + 127, 0, 255);
  analogWrite(AUDIO_PIN, sample);
}
