int blinkPin = 9; // LEDを接続するピン番号

void setup() {
  pinMode(blinkPin, OUTPUT); // blinkPinを出力に設定
}

void loop() {
  digitalWrite(blinkPin, HIGH); // LEDを点灯
  delay(200);                  // 1000ミリ秒待つ（1秒）
  digitalWrite(blinkPin, LOW);  // LEDを消灯
  delay(200);                  // 1000ミリ秒待つ（1秒）
}