#include <BluetoothSerial.h>
BluetoothSerial mybt;
String ms = "";
int touch_switch; // 터치 센서
unsigned long previousMillis = 0; // 이전 시간 저장 변수
const long interval = 500; // 터치 센서 체크 간격

void setup() {
  mybt.begin("Pebble_pal"); // 블루투스 이름 설정
  Serial.begin(115200); // 시리얼 통신 
  pinMode(2, OUTPUT); // LED 핀 설정
}

void loop() { 
  unsigned long currentMillis = millis(); // 현재 시간

  // 1초마다 터치 센서 읽기
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis; // 이전 시간을 현재 시간으로 업데이트
    touch_switch = touchRead(14); // 터치 센서 핀 읽기
    Serial.println(touch_switch); // 터치 반응 확인
    Serial.println("\n");
  }

  // Bluetooth 메시지 수신 처리
  if (mybt.available()) {
    ms = mybt.readStringUntil('c');
    if (ms == "1") {
      digitalWrite(2, HIGH); // LED 켜기
    } else {
      digitalWrite(2, LOW); // LED 끄기
    }
  }   
}
