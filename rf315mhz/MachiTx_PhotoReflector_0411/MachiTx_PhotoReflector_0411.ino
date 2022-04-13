#include <RH_ASK.h>
#include <SPI.h> // Not actually used but needed to compile

// tx/rx configuration
#define rxPin 2 // ATTiny, RX on D3 (pin 2 on attiny85)
#define txPin 9 // ATTiny, TX on D1 (pin 0 on attiny85)
#define txSpeed 2000

RH_ASK driver(txSpeed, rxPin, txPin);

bool Sensflag = 0;

void setup()
{
    pinMode(2,INPUT_PULLUP);
    pinMode(10,OUTPUT);
    digitalWrite(10,LOW);
    Serial.begin(9600);
    if (!driver.init())
         Serial.println("init failed");
}

void loop()
{
    Serial.println(analogRead(A1));
    
    const char *msg = "G";  //送るメッセージ
    
    if(analogRead(A1)<990 && Sensflag == 0){
    Sensflag=1;
    driver.send((uint8_t *)msg, strlen(msg));
    driver.waitPacketSent();
    Serial.print("sent ");
    Serial.println(msg);
    digitalWrite(10,HIGH);
    delay(200);
    digitalWrite(10,LOW);
    }else if(analogRead(A1)>1000){Sensflag=0;}
}
