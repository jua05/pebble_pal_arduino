#include <BLEDevice.h>
#include <Adafruit_NeoPixel.h>

#define SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

#define PIN 25
#define NUMPIXELS 8
Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

#define SHOCK 34
#define LED1 2

int r, g, b;
unsigned long previousMillis = 0;
const long interval = 500;

BLEServer* pServer = nullptr;
BLECharacteristic* pCharacteristic = nullptr;
bool deviceConnected = false;

// 함수 선언
void resetLED();
void setLEDColor(int r, int g, int b);

// BLE 서버 콜백
class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) override {
    deviceConnected = true;
    Serial.println("Device connected");
    pCharacteristic->setValue("connected");
    pCharacteristic->notify();
  }

  void onDisconnect(BLEServer* pServer) override {
    deviceConnected = false;
    Serial.println("Device disconnected");
    resetLED();  // LED 초기화 호출
    pCharacteristic->setValue("disconnected");
    pCharacteristic->notify();
    pServer->startAdvertising();
    Serial.println("Advertising restarted");
  }
};

// BLE 특성 콜백
class MyCharacteristicCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic* pCharacteristic) override {
    String value = pCharacteristic->getValue().c_str();
    Serial.println("Received value: " + value);

    if (value.length() > 0) {
      if (parseColor(value, r, g, b)) {
        Serial.printf("Parsed RGB values: R=%d, G=%d, B=%d\n", r, g, b);
        setLEDColor(r, g, b);  // LED 색상 설정 호출
      } else {
        Serial.println("Invalid color format");
        resetLED();  // 잘못된 데이터일 경우 LED 초기화
      }
    }
  }

  bool parseColor(const String& colorStr, int& r, int& g, int& b) {
    int firstComma = colorStr.indexOf(',');
    int secondComma = colorStr.indexOf(',', firstComma + 1);
    if (firstComma == -1 || secondComma == -1) {
      Serial.println("Error: Color string does not contain enough commas.");
      return false;
    }
    r = colorStr.substring(0, firstComma).toInt();
    g = colorStr.substring(firstComma + 1, secondComma).toInt();
    b = colorStr.substring(secondComma + 1).toInt();
    return true;
  }
};

// LED 제어 함수 구현
void resetLED() {
  pixels.clear();
  pixels.show();
  Serial.println("LED cleared");
}

void setLEDColor(int r, int g, int b) {
  for (int i = 0; i < NUMPIXELS; i++) {
    pixels.setPixelColor(i, pixels.Color(r, g, b));
  }
  pixels.show();
  Serial.printf("LED set to R:%d, G:%d, B:%d\n", r, g, b);
}

void setup() {
  pixels.begin();
  pinMode(LED1, OUTPUT);
  pinMode(SHOCK, INPUT);
  Serial.begin(115200);
  BLEDevice::init("DORI");

  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  BLEService* pService = pServer->createService(SERVICE_UUID);
  pCharacteristic = pService->createCharacteristic(
    CHARACTERISTIC_UUID,
    BLECharacteristic::PROPERTY_READ |
    BLECharacteristic::PROPERTY_WRITE |
    BLECharacteristic::PROPERTY_NOTIFY
  );
  pCharacteristic->setCallbacks(new MyCharacteristicCallbacks());

  pService->start();
  pServer->getAdvertising()->start();
  Serial.println("BLE advertising started");
}

void loop() {
  unsigned long currentMillis = millis();

  if (deviceConnected) {
    if (digitalRead(SHOCK) == HIGH) {
      if (currentMillis - previousMillis >= interval) {
        previousMillis = currentMillis;
        Serial.println("Shock detected");
        r = random(0, 256);
        g = random(0, 256);
        b = random(0, 256);
        setLEDColor(r, g, b);
        String colorValue = String(r) + "," + String(g) + "," + String(b);
        pCharacteristic->setValue(colorValue);
        pCharacteristic->notify();
      }
    }
  } else {
    resetLED();
  }
}
