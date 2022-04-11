#include <RH_ASK.h>
#include <SPI.h> // Not actually used but needed to compile

// tx/rx configuration
#define rxPin 2 // ATTiny, RX on D3 (pin 2 on attiny85)
#define txPin 9 // ATTiny, TX on D1 (pin 0 on attiny85)
#define txSpeed 2000

RH_ASK driver(txSpeed, rxPin, txPin);

void setup()
{
    pinMode(2,INPUT_PULLUP);
    pinMode(10,OUTPUT);
    Serial.begin(9600);   // Debugging only
    if (!driver.init())
         Serial.println("init failed");
}

void loop()
{
    const char *msg = "B";
    if(analogRead(A1)<400){
    driver.send((uint8_t *)msg, strlen(msg));
    driver.waitPacketSent();
    Serial.print("sent ");
    Serial.println(msg);
    digitalWrite(10,HIGH);
    delay(100);
    digitalWrite(10,LOW);
    }
}
