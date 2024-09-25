#include <BluetoothSerial.h>
BluetoothSerial mybt;
String ms ="";
void setup() {
  mybt.begin("ESP32 Bluetooth0107");
  pinMode(2, OUTPUT);
}


void loop() {
  if(mybt.available()){
    ms = mybt.readStringUntil('c');
    if(ms=="1"){
      digitalWrite(2,HIGH);
    }else{
      digitalWrite(2,LOW);
    }
  }          
}
