#include <bluefruit.h>

// ── 핀 정의 ──────────────────────────────────────
#define LIGHT_PIN 31
#define DIP_R     19
#define DIP_G     20
#define DIP_B     21

// ── UUID 정의 ─────────────────────────────────────
#define SERVICE_UUID      "12345678-1234-1234-1234-123456789000"
#define SENSOR_UUID       "12345678-1234-1234-1234-123456789001"
#define LED_CONTROL_UUID  "12345678-1234-1234-1234-123456789002"
#define AUTO_MODE_UUID    "12345678-1234-1234-1234-123456789003"

// ── BLE 객체 ──────────────────────────────────────
BLEService        svc(SERVICE_UUID);
BLECharacteristic sensorChar(SENSOR_UUID);
BLECharacteristic ledChar(LED_CONTROL_UUID);
BLECharacteristic autoChar(AUTO_MODE_UUID);

// ── 상태 변수 ─────────────────────────────────────
bool autoMode = true;
int  manualBrightness = 0;
unsigned long lastNotify = 0;

// ── 조도 상태 판단 ────────────────────────────────
String getLightStatus(int val) {
  if (val >= 300) return "어두움";
  if (val >= 150) return "적당";
  return "밝음";
}

// ── LED 밝기 적용 ─────────────────────────────────
void applyLED(int brightness) {
  analogWrite(DIP_R, brightness);
  analogWrite(DIP_G, brightness);
  analogWrite(DIP_B, brightness);
}

// ── LED_CONTROL 수신 콜백 ─────────────────────────
void onLedWrite(uint16_t conn, BLECharacteristic* chr, 
                uint8_t* data, uint16_t len) {
  if (len > 0) {
    manualBrightness = data[0];  // 0~255
    if (!autoMode) applyLED(manualBrightness);
  }
}

// ── AUTO_MODE 수신 콜백 ───────────────────────────
void onAutoWrite(uint16_t conn, BLECharacteristic* chr,
                 uint8_t* data, uint16_t len) {
  if (len > 0) {
    autoMode = (data[0] == 1);
  }
}

// ── BLE 연결/해제 콜백 ───────────────────────────
void onConnect(uint16_t conn) {
  digitalWrite(PIN_LED1, LOW);   // 연결 시 LED1 ON
}
void onDisconnect(uint16_t conn, uint8_t reason) {
  digitalWrite(PIN_LED1, HIGH);  // 해제 시 LED1 OFF
  Bluefruit.Advertising.start(0);
}

// ── BLE 초기화 ────────────────────────────────────
void setupBLE() {
  Bluefruit.begin();
  Bluefruit.setName("NU40-SMARTHOME");
  Bluefruit.Periph.setConnectCallback(onConnect);
  Bluefruit.Periph.setDisconnectCallback(onDisconnect);

  svc.begin();

  // SENSOR — Notify
  sensorChar.setProperties(CHR_PROPS_NOTIFY);
  sensorChar.setPermission(SECMODE_OPEN, SECMODE_NO_ACCESS);
  sensorChar.setMaxLen(64);
  sensorChar.begin();

  // LED_CONTROL — Write
  ledChar.setProperties(CHR_PROPS_WRITE | CHR_PROPS_WRITE_WO_RESP);
  ledChar.setPermission(SECMODE_OPEN, SECMODE_OPEN);
  ledChar.setMaxLen(1);
  ledChar.setWriteCallback(onLedWrite);
  ledChar.begin();

  // AUTO_MODE — Write
  autoChar.setProperties(CHR_PROPS_WRITE | CHR_PROPS_WRITE_WO_RESP);
  autoChar.setPermission(SECMODE_OPEN, SECMODE_OPEN);
  autoChar.setMaxLen(1);
  autoChar.setWriteCallback(onAutoWrite);
  autoChar.begin();

  // Advertising
  Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
  Bluefruit.Advertising.addTxPower();
  Bluefruit.Advertising.addService(svc);
  Bluefruit.ScanResponse.addName();
  Bluefruit.Advertising.restartOnDisconnect(true);
  Bluefruit.Advertising.setInterval(32, 244);
  Bluefruit.Advertising.setFastTimeout(30);
  Bluefruit.Advertising.start(0);
}

void setup() {
  pinMode(DIP_R, OUTPUT);
  pinMode(DIP_G, OUTPUT);
  pinMode(DIP_B, OUTPUT);
  pinMode(PIN_LED1, OUTPUT);
  digitalWrite(PIN_LED1, HIGH);  // OFF

  applyLED(0);
  setupBLE();
}

void loop() {
  int lightVal = analogRead(LIGHT_PIN);

  // ── 자동 모드일 때 LED 밝기 자동 조절 ────────────
  if (autoMode) {
    int brightness = map(lightVal, 80, 500, 0, 255);
    brightness = constrain(brightness, 0, 255);
    applyLED(brightness);
  }

  // ── 500ms마다 BLE Notify ──────────────────────────
  if (millis() - lastNotify >= 500) {
    lastNotify = millis();

    if (Bluefruit.connected()) {
      String status = getLightStatus(lightVal);
      String json = "{\"light\":" + String(lightVal) + 
                    ",\"status\":\"" + status + "\"}";

      sensorChar.notify((uint8_t*)json.c_str(), json.length());
    }
  }
}