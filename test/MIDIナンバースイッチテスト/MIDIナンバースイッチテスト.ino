int beforeSWState = 0;
unsigned long beforeMillis=0;

int period =500; //長押しの長さ
int midinumber=0; //midiの番号、これで出す音変える

void setup() {
  pinMode(2,INPUT_PULLUP);
  Serial.begin(9600);
}

void loop() {

  unsigned long nowMillis=millis();
  int SwState = digitalRead(2);

  if(SwState==0&&beforeSWState==1){ //スイッチ押したとき
    beforeSWState=0;
    Serial.println("Push");
    beforeMillis=millis();
  }
    if(SwState==1 && beforeSWState==0){ //スイッチ離したとき
    midinumber++;
    beforeSWState=1;
    Serial.println("Release");
    
    if(nowMillis-beforeMillis <= period){//短い押し
    Serial.Println(midinumber);
    }
    if(nowMillis-beforeMillis >= period){ //長押し
    }
  }

}