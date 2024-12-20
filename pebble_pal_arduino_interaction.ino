#include <BLEDevice.h>
#include <Adafruit_NeoPixel.h>

#define SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

#define PIN 25
#define NUMPIXELS 8
Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

#define SHOCK 34
#define LED1 2

unsigned long previousMillis = 0;
const long colorInterval = 300;

BLEServer* pServer = nullptr;
BLECharacteristic* pCharacteristic = nullptr;
bool deviceConnected = false;

int r, g, b;
int rainbowColors[][3] = {
  {255, 0, 0},   
  {255, 127, 0}, 
  {255, 255, 0}, 
  {0, 255, 0},   
  {0, 0, 255},   
  {75, 0, 130},  
  {148, 0, 211}  
};

// 함수 선언
void resetLED();
void setLEDColor(int r, int g, int b);
void startRainbowSequence();

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
      if (value == "2") {
        Serial.println("Starting rainbow sequence");
        startRainbowSequence();
      } else {
        Serial.println("Unknown value received");
      }
    }
  }
};

// LED 초기화 함수
void resetLED() {
  pixels.clear();
  pixels.show();
  Serial.println("LED cleared");
}

// LED 색상 설정 함수
void setLEDColor(int r, int g, int b) {
  for (int i = 0; i < NUMPIXELS; i++) {
    pixels.setPixelColor(i, pixels.Color(r, g, b));
  }
  pixels.show();
  Serial.printf("LED set to R:%d, G:%d, B:%d\n", r, g, b);
}


void startRainbowSequence() {
  for (int i = 0; i < 7; i++) { 
    setLEDColor(rainbowColors[i][0], rainbowColors[i][1], rainbowColors[i][2]);
    delay(colorInterval);
  }
  resetLED(); 
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
      if (currentMillis - previousMillis >= 500) {
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
