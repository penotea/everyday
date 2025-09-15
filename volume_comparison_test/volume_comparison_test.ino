/*
 * Volume Comparison Test - tone()関数との音量比較テスト
 * 
 * 機能:
 * - tone()関数とPWMエンベロープ付き音声の音量比較
 * - ボタンで切り替えて音量差を確認
 * 
 * ピン配置:
 * - 9: 音声出力 (PWM/tone共用)
 * - 2: 切り替えボタン (LOW=tone(), HIGH=PWMエンベロープ)
 * - 3: トリガーボタン (LOW=音声再生)
 */

#define AUDIO_PIN 9
#define MODE_BUTTON_PIN 2
#define TRIGGER_BUTTON_PIN 3

// tone()モード用
bool useToneMode = true;
bool lastModeButtonState = HIGH;
bool lastTriggerButtonState = HIGH;

// PWMエンベロープモード用
const float FREQUENCY = 440.0; // A4音
const int SINE_TABLE_SIZE = 256;
int sineTable[SINE_TABLE_SIZE];
float phaseAccumulator = 0;
float phaseIncrement;
const float SAMPLE_RATE = 8000.0;

// エンベロープ設定
const unsigned long ATTACK_TIME = 50;   // 短いアタック
const unsigned long SUSTAIN_TIME = 500; // サステイン時間
const unsigned long RELEASE_TIME = 200; // 短いリリース

struct EnvelopeState {
  bool active;
  unsigned long startTime;
  float amplitude;
  enum Phase { ATTACK, SUSTAIN, RELEASE, IDLE } phase;
} envelope;

void setup() {
  Serial.begin(9600);
  
  // ピン設定
  pinMode(AUDIO_PIN, OUTPUT);
  pinMode(MODE_BUTTON_PIN, INPUT_PULLUP);
  pinMode(TRIGGER_BUTTON_PIN, INPUT_PULLUP);
  
  // サイン波テーブル生成（tone()と同じ音量レベル）
  generateSineTable();
  
  // 位相増分計算
  phaseIncrement = (FREQUENCY * SINE_TABLE_SIZE) / SAMPLE_RATE;
  
  // エンベロープ初期化
  envelope.active = false;
  envelope.amplitude = 0;
  envelope.phase = EnvelopeState::IDLE;
  
  // タイマー1設定
  setupTimer1();
  
  Serial.println("Volume Comparison Test Ready");
  Serial.println("Mode Button: tone() <-> PWM Envelope");
  Serial.println("Trigger Button: Play sound");
  Serial.print("Current mode: ");
  Serial.println(useToneMode ? "tone()" : "PWM Envelope");
}

void loop() {
  // モード切り替えボタン
  bool modeButtonState = digitalRead(MODE_BUTTON_PIN);
  if (modeButtonState == LOW && lastModeButtonState == HIGH) {
    useToneMode = !useToneMode;
    Serial.print("Mode changed to: ");
    Serial.println(useToneMode ? "tone()" : "PWM Envelope");
    
    // tone()を停止（もし再生中なら）
    if (useToneMode) {
      noTone(AUDIO_PIN);
    }
    delay(200); // チャタリング防止
  }
  lastModeButtonState = modeButtonState;
  
  // トリガーボタン
  bool triggerButtonState = digitalRead(TRIGGER_BUTTON_PIN);
  if (triggerButtonState == LOW && lastTriggerButtonState == HIGH) {
    playSound();
    delay(200); // チャタリング防止
  }
  lastTriggerButtonState = triggerButtonState;
  
  // PWMモードの場合のエンベロープ更新
  if (!useToneMode) {
    updateEnvelope();
  }
  
  delay(10);
}

void playSound() {
  if (useToneMode) {
    // tone()関数で再生
    Serial.println("Playing with tone()");
    tone(AUDIO_PIN, FREQUENCY, 750); // 750ms再生
  } else {
    // PWMエンベロープで再生
    Serial.println("Playing with PWM Envelope");
    envelope.active = true;
    envelope.startTime = millis();
    envelope.phase = EnvelopeState::ATTACK;
    envelope.amplitude = 0;
    phaseAccumulator = 0;
  }
}

void updateEnvelope() {
  if (!envelope.active) return;
  
  unsigned long currentTime = millis();
  unsigned long elapsed = currentTime - envelope.startTime;
  
  switch (envelope.phase) {
    case EnvelopeState::ATTACK:
      if (elapsed < ATTACK_TIME) {
        envelope.amplitude = (float)elapsed / ATTACK_TIME;
      } else {
        envelope.phase = EnvelopeState::SUSTAIN;
        envelope.startTime = currentTime;
      }
      break;
      
    case EnvelopeState::SUSTAIN:
      envelope.amplitude = 1.0;
      if (elapsed >= SUSTAIN_TIME) {
        envelope.phase = EnvelopeState::RELEASE;
        envelope.startTime = currentTime;
      }
      break;
      
    case EnvelopeState::RELEASE:
      if (elapsed < RELEASE_TIME) {
        float progress = (float)elapsed / RELEASE_TIME;
        envelope.amplitude = 1.0 - progress;
      } else {
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
    // tone()と同じ音量レベル（約25%の振幅）
    sineTable[i] = (int)(64 * sin(angle));
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

// タイマー1割り込みハンドラ（PWM音声生成）
ISR(TIMER1_COMPA_vect) {
  int sample = 0;
  
  if (!useToneMode && envelope.active && envelope.amplitude > 0) {
    // サイン波生成
    int tableIndex = (int)phaseAccumulator % SINE_TABLE_SIZE;
    sample = sineTable[tableIndex];
    
    // エンベロープ適用
    sample = (int)(sample * envelope.amplitude);
    
    // 位相更新
    phaseAccumulator += phaseIncrement;
    if (phaseAccumulator >= SINE_TABLE_SIZE) {
      phaseAccumulator -= SINE_TABLE_SIZE;
    }
  }
  
  // tone()モードの時はPWM出力を停止
  if (useToneMode) {
    return;
  }
  
  // 出力範囲調整とPWM出力
  sample = constrain(sample + 127, 0, 255);
  analogWrite(AUDIO_PIN, sample);
}
