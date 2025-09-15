#include <RH_ASK.h>
#include <SPI.h>  // Not actually used but needed to compile
#include "PhotoReflectorModule.h"
#include <Adafruit_NeoPixel.h>

// 無線モジュールの設定
#define rxPin 2
#define txPin 9
#define txSpeed 1000

RH_ASK driver(txSpeed, rxPin, txPin, false);

// NeoPixelの設定
#define PIN 11
#define NUMPIXELS 1
Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

// センサーの設定
const int sensorNum = 3;
PhotoReflectorModule sensors[sensorNum];

// DIPスイッチの設定
int DIPSw[4];
int SumDipSw;

void setup() {
  Serial.begin(9600);
  if (!driver.init()) Serial.println("init failed");

  // NeoPixelの初期化
  pixels.begin();
  pixels.setPixelColor(0, pixels.Color(100, 100, 100));
  pixels.show();
  delay(1000);
  pixels.clear();

  // センサーの初期化
  sensors[0].setup(12, A1);
  sensors[1].setup(10, A2);
  sensors[2].setup(8, A3);

  // DIPスイッチの設定
  pinMode(2, INPUT_PULLUP);
  pinMode(3, INPUT_PULLUP);
  pinMode(4, INPUT_PULLUP);
  pinMode(5, INPUT_PULLUP);

  DIPSwRecognize();
}

void loop() {
  DIPSwRecognize();
  sensorsUpdate();
  processEvents();
}

// DIPスイッチを認識する関数
void DIPSwRecognize() {
  DIPSw[0] = !digitalRead(2);
  DIPSw[1] = !digitalRead(3);
  DIPSw[2] = !digitalRead(4);
  DIPSw[3] = !digitalRead(5);
  
  if (DIPSw[0] == 0) DIPSw[0] = 2;
  SumDipSw = DIPSw[0] * 1000 + DIPSw[1] * 100 + DIPSw[2] * 10 + DIPSw[3];
}

// センサーの状態を更新する関数
void sensorsUpdate() {
  // センサーの読み取りとLEDの制御を行う
  for (int i = 0; i < sensorNum; ++i) {
    sensors[i].update();
  }
}

// イベントを処理する関数
void processEvents() {
  static unsigned long prevTime = 0;
  unsigned long currentTime = millis();

  // 一定間隔でセンサーの状態をチェック
  if (currentTime - prevTime > 200) {
    prevTime = currentTime;
    
    // センサーがトリガーされた場合の処理
    for (int i = 0; i < sensorNum; ++i) {
      if (sensors[i].isTriggered()) {
        sendMsg(i);  // トリガーされたセンサーに対応するメッセージを送信
        break;  // 一度に一つのイベントのみを処理
      }
    }
  }
}

// メッセージを送信する関数
void sendMsg(int sensorId) {
  // DIPスイッチの値に基づいて送信するメッセージを選択
  uint8_t msg;
  switch (SumDipSw) {
    case 1000:
    case 1100:
    case 1110:
    case 1111:
      msg = sensorId;  /*
      ここで適切なメッセージを設定します。
      実際には、ここで sensorId に基づいて、または SumDipSw の値に基づいて、実際に送信するメッセージを決定します。
      この例では、シンプルさのために sensorId をそのままメッセージとしていますが、
      実際の用途に合わせて適切なメッセージにマッピングする必要があります。 */

      // 例: SumDipSwの値に応じたメッセージの選択
      if (SumDipSw == 1000) msg = 0xA0 + sensorId; // 例えば、A0, A1, A2...
      else if (SumDipSw == 1100) msg = 0xB0 + sensorId;
      else if (SumDipSw == 1110) msg = 0xC0 + sensorId;
      else if (SumDipSw == 1111) msg = 0xD0 + sensorId;
      // さらに条件分岐が必要な場合はここに追加
      break;
    // 他のDIPスイッチの値に基づくケースを追加
    default:
      msg = 0xFF; // デフォルトメッセージ（エラーまたは未定義の動作）
  }

  // メッセージを送信
  driver.send(&msg, sizeof(msg));
  driver.waitPacketSent();

  // NeoPixelまたは他のフィードバックメカニズムを通じてフィードバックを提供
  // 例: NeoPixelで一定時間光らせる
  pixels.setPixelColor(0, pixels.Color(0, 150, 0)); // 緑色
  pixels.show();
  delay(200);
  pixels.clear();
}

