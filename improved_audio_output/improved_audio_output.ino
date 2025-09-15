/*
 * Improved Audio Output - PWM周波数改善版
 * 
 * 改善点:
 * - PWM周波数を31.25kHzに変更（可聴域外）
 * - より大きな振幅でスピーカー駆動
 * - ACカップリング対応
 * 
 * ピン配置:
 * - 9: 音声出力 (高周波PWM)
 * - 2: DIPスイッチ1 (HIGH=Sine波, LOW=Square波)
 * 
 * 動作: 一定周期ごとに自動で鐘音を鳴らします
 */

#define AUDIO_PIN 9
#define DIP_SWITCH_1_PIN 2  // 波形選択(sine/square)

// 音の種類管理と周期制御
bool useSineWave = true;  // true: sine, false: square
unsigned long lastBellTime = 0;
const unsigned long BELL_INTERVAL = 3000; // 3秒間隔

// 音声生成用
const int FREQUENCY = 6440; // A5音
const int SINE_TABLE_SIZE = 64; // テーブルサイズ
int sineTable[SINE_TABLE_SIZE];
int phaseAccumulator = 0;
int phaseIncrement;
const int SAMPLE_RATE = 440; // 2kHzサンプリング

// エンベロープ設定（鐘のような減衰音）
const unsigned long ATTACK_TIME = 5;    // 瞬間的なアタック
const unsigned long SUSTAIN_TIME = 0;   // サステインなし
const unsigned long RELEASE_TIME = 1000; // 長い減衰（2秒） 

struct EnvelopeState {
  bool active;
  unsigned long startTime;
  float amplitude;
  enum Phase { ATTACK, SUSTAIN, RELEASE, IDLE } phase;
} envelope;

void setup() {
  // Serial.begin(9600);
  
  // ピン設定
  pinMode(AUDIO_PIN, OUTPUT);
  pinMode(DIP_SWITCH_1_PIN, INPUT_PULLUP);
  
  // PWM周波数を31.25kHzに変更（可聴域外）
  setupHighFrequencyPWM();
  
  // サイン波テーブル生成（より大きな振幅）
  generateSineTable();
  
  // 位相増分計算（整数演算）
  phaseIncrement = (FREQUENCY * SINE_TABLE_SIZE) / SAMPLE_RATE;
  
  // エンベロープ初期化
  envelope.active = false;
  envelope.amplitude = 0;
  envelope.phase = EnvelopeState::IDLE;
  
  // 初期設定完了
  
  // Serial.println("Automatic Bell Generator Ready");
  // Serial.println("DIP Switch 1 (Pin 2): Sine/Square wave selection");
  // Serial.println("Bell rings automatically every 3 seconds");
  // printCurrentSettings();
}

void loop() {
  // DIPスイッチの状態を確認
  bool dipSwitch1 = digitalRead(DIP_SWITCH_1_PIN);  // sine/square
  
  // 音の種類を更新
  useSineWave = (dipSwitch1 == HIGH);  // HIGH=sine, LOW=square
  
  // 一定周期ごとに音を鳴らす
  unsigned long currentTime = millis();
  if (currentTime - lastBellTime >= BELL_INTERVAL) {
    playSound();
    lastBellTime = currentTime;
  }
  
  // エンベロープ更新と音声生成
  updateEnvelope();
  generatePWMAudio();
  
  delay(1);  // より短い遅延でレスポンシブに
}

void setupHighFrequencyPWM() {
  // Timer1のPWM周波数を31.25kHzに設定（ピン9用）
  // これにより、PWMキャリア周波数が可聴域外になり、
  // スピーカーでより良い音質が得られる
  TCCR1A = _BV(COM1A1) | _BV(WGM10);  // Fast PWM, 8-bit, OCR1A出力
  TCCR1B = _BV(WGM12) | _BV(CS10);    // Fast PWM, プリスケーラ1
}

void playSound() {
  // Serial.print("Ringing ");
  // Serial.print(useSineWave ? "Sine" : "Square");
  // Serial.println(" Bell");
  
  // 前の音を完全に停止
  envelope.active = false;
  envelope.amplitude = 0;
  OCR1A = 127;  // 中央値で無音状態
  
  // 短い無音期間を作る
  delay(10);
  
  setupHighFrequencyPWM(); // PWM設定
  envelope.active = true;
  envelope.startTime = millis();
  envelope.phase = EnvelopeState::ATTACK;
  envelope.amplitude = 0;
  phaseAccumulator = 0;
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
        // アタック完了後、すぐにリリースフェーズへ（鐘のような音）
        envelope.phase = EnvelopeState::RELEASE;
        envelope.startTime = currentTime;
        envelope.amplitude = 1.0;
      }
      break;
      
    case EnvelopeState::SUSTAIN:
      // サステインフェーズは使用しない（鐘は持続しない）
      envelope.phase = EnvelopeState::RELEASE;
      envelope.startTime = currentTime;
      break;
      
    case EnvelopeState::RELEASE:
      if (elapsed < RELEASE_TIME) {
        float progress = (float)elapsed / RELEASE_TIME;
        // 指数的減衰をシミュレート（鐘らしい減衰カーブ）
        envelope.amplitude = (1.0 - progress) * (1.0 - progress);
        
        // 振幅が非常に小さくなったら強制終了
        if (envelope.amplitude < 0.01) {
          envelope.active = false;
          envelope.amplitude = 0;
          envelope.phase = EnvelopeState::IDLE;
        }
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
    // スピーカー駆動のため、より大きな振幅を使用
    sineTable[i] = (int)(127 * sin(angle)); // 振幅を127に調整
  }
}

void setupTimer1() {
  // PWMモード時のみTimer1のFast PWMを設定
  // tone()使用時は標準のTimer設定を維持
}

// PWM音声生成関数（割り込みではなくloop内で呼び出し）
void generatePWMAudio() {
  static unsigned long lastUpdate = 0;
  unsigned long currentTime = micros();
  
  // 8kHzサンプリング（125マイクロ秒間隔）
  if (currentTime - lastUpdate < 125) {
    return;
  }
  lastUpdate = currentTime;
  
  int sample = 0;
  
  if (envelope.active && envelope.amplitude > 0) {
    if (useSineWave) {
      // サイン波生成
      int tableIndex = (int)phaseAccumulator % SINE_TABLE_SIZE;
      sample = sineTable[tableIndex];
    } else {
      // 方形波生成
      int tableIndex = (int)phaseAccumulator % SINE_TABLE_SIZE;
      sample = (tableIndex < SINE_TABLE_SIZE/2) ? 127 : -127;
    }
    
    // エンベロープ適用
    sample = (int)(sample * envelope.amplitude);
    
    // 位相更新（整数演算）
    phaseAccumulator += phaseIncrement;
    if (phaseAccumulator >= SINE_TABLE_SIZE) {
      phaseAccumulator -= SINE_TABLE_SIZE;
    }
  }
  
  // エンベロープが無効の時はPWM出力を停止
  if (!envelope.active) {
    OCR1A = 127;  // 中央値で無音状態
    return;
  }
  
  // 振幅が非常に小さい場合も無音にする
  if (envelope.amplitude < 0.01) {
    OCR1A = 127;  // 中央値で無音状態
    return;
  }
  
  // 出力範囲調整とPWM出力
  sample = constrain(sample + 127, 0, 255);  // 8-bit PWM用に調整
  
  // 高周波PWMで出力
  OCR1A = sample;
}


void printCurrentSettings() {
  // Serial.print("Wave type: ");
  // Serial.println(digitalRead(DIP_SWITCH_1_PIN) == HIGH ? "Sine" : "Square");
  // Serial.println("Auto-bell every 3 seconds");
}
