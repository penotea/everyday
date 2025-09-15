#include <MIDIUSB.h>
#include <RH_ASK.h>
#include <SPI.h> // Not actually used but needed to compile

// tx/rx configuration
#define rxPin 2 // ATTiny, RX on D3 (pin 2 on attiny85)
#define txPin 0 // ATTiny, TX on D1 (pin 0 on attiny85)
#define txSpeed 1000

RH_ASK driver(txSpeed, rxPin, txPin, false);

// LEDピンの設定
const int ledPins[] = {A2, A1, A0, 15, 14, 16, A10, 9}; // 受信した番号に対応するLEDのピン
const int numLeds = sizeof(ledPins) / sizeof(ledPins[0]);

int beforestate=0;  //前のときにセンシングしたかどうか

void setup() {
  Serial.begin(9600); // For debugging
  if (!driver.init()) Serial.println("init failed");

  // LEDピンを出力として設定
  for (int i = 0; i < numLeds; i++) {
    pinMode(ledPins[i], OUTPUT);
  }
}

void loop() {
  uint8_t buf[RH_ASK_MAX_MESSAGE_LEN];
  uint8_t buflen = sizeof(buf);

  if (driver.recv(buf, &buflen) /*&& beforestate==0&& buflen == 1*/) { // 1バイトのデータを期待
    beforestate=1;
    uint8_t receivedValue = buf[0];

    // 受信した値をシリアル出力
    Serial.print("Received: ");
    Serial.println(receivedValue);

    // MIDIメッセージを送信
    if(receivedValue>=100){
    uint8_t note = 35 + receivedValue-100; // C1からのオフセットを計算
    if(note==37)note=40;
    if(note==38)note=43;

    if(note==39)note=38;
    if(note==40)note=45;
    if(note==41)note=48;

    if(note==42)note=48;
    if(note==43)note=52;
    if(note==44)note=55;

    //Serial.print("note: ");
    //Serial.println(note);
    digitalWrite(ledPins[receivedValue%8], HIGH);
    noteOn(0, note, 127); // ノートオン
    delay(40); // 一定時間待機
    noteOff(0, note, 0); // ノートオフ
    digitalWrite(ledPins[receivedValue%8], LOW);
    }
    
    else if(receivedValue<=100){
    uint8_t note = 36 + receivedValue; // C1からのオフセットを計算
    //Serial.print("note: ");
    //Serial.println(note);
    digitalWrite(ledPins[receivedValue%8], HIGH);
    noteOn(1, note, 127); // ノートオン
    delay(40); // 一定時間待機
    noteOff(1, note, 0); // ノートオフ
    digitalWrite(ledPins[receivedValue%8], LOW);
    }

    /*
    // 対応するLEDを点灯、存在しない場合は何もしない
    if (receivedValue < numLeds) {
      digitalWrite(ledPins[receivedValue], HIGH);
      delay(300); // 0.3秒間待機
      digitalWrite(ledPins[receivedValue], LOW);
      digitalWrite(ledPins[receivedValue], LOW);
    }
    */
  }else{beforestate=0;}
  /*
  else if(driver.recv(buf, &buflen)){ // 途中でデータ受信が途切れた場合
      Serial.println("Error: Incomplete data received.");
      digitalWrite(ledPins[1], HIGH);
      digitalWrite(ledPins[3], HIGH);
      delay(80); // 一定時間待機
      digitalWrite(ledPins[1], LOW);
      digitalWrite(ledPins[3], LOW);
      Serial.print("buflen: ");
    Serial.println(buflen);
    }
    */
}

void noteOn(uint8_t channel, uint8_t note, uint8_t velocity) {
  midiEventPacket_t noteOn = {0x09, 0x90 | channel, note, velocity};
  MidiUSB.sendMIDI(noteOn);
  MidiUSB.flush(); // 送信を確実に行う
}

void noteOff(uint8_t channel, uint8_t note, uint8_t velocity) {
  midiEventPacket_t noteOff = {0x08, 0x80 | channel, note, velocity};
  MidiUSB.sendMIDI(noteOff);
  MidiUSB.flush(); // 送信を確実に行う
}
