#include <BluetoothSerial.h>
BluetoothSerial mybt; // 블루투스
String ms = ""; // 블루투스 입력 값 변수

#include <Adafruit_NeoPixel.h>
#define PIN 25 // LED 핀
#define NUMPIXELS 8 // NeoPixel 링 크기
Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800); // 개수, 핀번호, 픽셀색상순서, 데이터 전송 속도

#define SHOCK 34 // 충격 센서 핀 번호

#define TOUCH 20 // 터치 센서 감도
int touch; // 터치 센서
unsigned long previousMillis = 0; // 이전 시간 저장 변수
const long interval = 500; // 터치 센서 체크 간격

void setup() {
  mybt.begin("Pebble_pal"); // 블루투스 이름 설정
  Serial.begin(115200); // 시리얼 통신 

  pixels.begin(); // NeoPixel 스트립 객체 초기화 (필수)

  pinMode(SHOCK, INPUT); // 쇼크 핀 
}

void loop() { 
  unsigned long currentMillis = millis(); // 현재 시간

  // 블루투스가 연결되었을 때만 동작
  if (mybt.connected()) {
    // 500ms마다 터치 센서 읽기
    if (currentMillis - previousMillis >= interval) {
      previousMillis = currentMillis; // 이전 시간을 현재 시간으로 업데이트
      touch = touchRead(14); // 터치 센서 핀 읽기
      
      if (touch < TOUCH) {
        mybt.println("read Touch"); // 휴대폰으로 블루투스 신호 전송
        if (ms == "2") {
          for (int i = 0; i < NUMPIXELS; i++) {
            pixels.setPixelColor(i, pixels.Color(0, 150, 0));
          }
          pixels.show(); // 업데이트를 한번만 호출
          ms = "1"; // 상태 업데이트
        } else {
          pixels.clear(); // 모든 픽셀 색상을 '꺼짐'으로 설정
          pixels.show();
          ms = "2"; // 상태 업데이트
        }
      }
      Serial.println(touch); // 터치 반응 확인
      Serial.println("\n");
    }

    // 충격 센서 감지 시
    if (digitalRead(SHOCK) == HIGH) {
      Serial.println("SHOCK!!!!");
      mybt.println("read shock");
    }

    // Bluetooth 메시지 수신 처리
    if (mybt.available()) {
      ms = mybt.readStringUntil('c');
      Serial.println(ms);
      if (ms == "1") {
        for (int i = 0; i < NUMPIXELS; i++) {
          pixels.setPixelColor(i, pixels.Color(0, 150, 0));
        }
        pixels.show();
      } else {
        pixels.clear(); // 모든 픽셀 색상을 '꺼짐'으로 설정
        pixels.show();
      }
    }
  } else {
      pixels.clear(); // 모든 픽셀 색상을 '꺼짐'으로 설정
      pixels.show();
  }
}
