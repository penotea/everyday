void setup() {
  // put your setup code here, to run once:

}

void loop() {


  for(int i=0;i<1000;i++){
    tone(9,i);
    delay(10);
  }

    for(int i=1000;i>0;i--){
    tone(9,i);
    delay(10);
  }


  
}
