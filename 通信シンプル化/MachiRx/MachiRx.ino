#include <RH_ASK.h>
#include "MIDIUSB.h"

#define ledPin 9 // ATTiny build in LED

// tx/rx configuration
#define rxPin 2 // ATTiny, RX on D3 (pin 2 on attiny85)
#define txPin 0 // ATTiny, TX on D1 (pin 0 on attiny85)
#define txSpeed 1000

RH_ASK driver(txSpeed, rxPin, txPin,false);

int delaymillis = 10;

void setup() {
    Serial.begin(9600);
    if (!driver.init()) {
        Serial.println("init failed");
        digitalWrite(ledPin, HIGH); // turn the LED on (HIGH is the voltage level)
        delay(10000); // wait for 10 seconds
    } else {
        Serial.println("ready");
    }

    pinMode(ledPin, OUTPUT);
}

void loop() {
    uint8_t buf[1] = {0};
    uint8_t buflen = sizeof(buf); 

    if (driver.recv(buf, &buflen)) { // Non-blocking
        uint8_t receivedMsg = buf[0]; // 受信したメッセージID
        Serial.print("Received message ID: ");
        Serial.println(receivedMsg);

        // MIDIイベントの生成
        generateMIDIEvent(receivedMsg);

        MidiUSB.flush(); // MIDIイベントを送信
        digitalWrite(ledPin, HIGH);
        delay(delaymillis);
        digitalWrite(ledPin, LOW);
    }
}

void generateMIDIEvent(uint8_t msgID) {
    // ここでは、msgIDに基づいてMIDIメッセージを生成します。
    // 例として、msgID 0から23までの範囲で24の異なるノートを鳴らすことができます。
    // msgIDに応じてMIDIチャンネル、ピッチ、ベロシティを設定
    uint8_t channel = 0;
    uint8_t pitch = 60 + (msgID % 24); // 60(C4)を基点として24ノートを循環
    uint8_t velocity = 100; // ベロシティは固定値

    noteOn(channel, pitch, velocity);
    delay(delaymillis); // ノートオン後、指定されたミリ秒数待機
    noteOff(channel, pitch, velocity);
}

void noteOn(byte channel, byte pitch, byte velocity) {
    midiEventPacket_t noteOn = {0x09, 0x90 | channel, pitch, velocity};
    MidiUSB.sendMIDI(noteOn);
}

void noteOff(byte channel, byte pitch, byte velocity) {
    midiEventPacket_t noteOff = {0x08, 0x80 | channel, pitch, velocity};
    MidiUSB.sendMIDI(noteOff);
}
