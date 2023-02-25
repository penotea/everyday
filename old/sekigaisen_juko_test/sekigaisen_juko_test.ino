//Lesson15 赤外線受信センサモジュールサンプルコード
//https://omoroya.com

#include <IRremote.h>

int RECV_PIN = 5; //デジタルIOの11番ピンを受信データ入力用に設定
const int LedPin = 10;

unsigned long startTime;  
unsigned long currentTime;
const unsigned long period = 100;

IRrecv irrecv(RECV_PIN); // 受信で使用するオブジェクトを作成 'irrecv'
decode_results results;  // 受信情報の格納先を作成 'results'

void setup(){
Serial.begin(9600);
irrecv.enableIRIn();     // 作成したオブジェクトで赤外線受信をスタート
pinMode(LedPin,OUTPUT);

startTime = millis();  //initial start time
}

void loop() {
currentTime = millis(); //時計
int LedState=0;

if (irrecv.decode(&results)) {      // 受信したかどうかの確認 未受信＝0/受信＝1
Serial.println(results.value, HEX); //16進表示
irrecv.resume();                    // .decode()の返り値をリセット
LedState=1;
}else{LedState=0;}


if(LedState){
if (currentTime - startTime >= period)//test whether the period has elapsed
    {
        digitalWrite(LedPin, !digitalRead(LedPin));//if so, change the state of the LED.
        startTime = currentTime;
    }
}

//delay(100);
}
