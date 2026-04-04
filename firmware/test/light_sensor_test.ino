#include <bluefruit.h>

BLEUart bleuart;

#define LIGHT_PIN 31
#define DIP_R     19
#define DIP_G     20
#define DIP_B     21

unsigned long lastSend = 0;

void setup() {
  Bluefruit.begin();
  Bluefruit.setName("NU40-LIGHT");

  bleuart.begin();

  Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
  Bluefruit.Advertising.addTxPower();
  Bluefruit.Advertising.addService(bleuart);
  Bluefruit.ScanResponse.addName();
  Bluefruit.Advertising.restartOnDisconnect(true);
  Bluefruit.Advertising.setInterval(32, 244);
  Bluefruit.Advertising.setFastTimeout(30);
  Bluefruit.Advertising.start(0);

  pinMode(DIP_R, OUTPUT);
  pinMode(DIP_G, OUTPUT);
  pinMode(DIP_B, OUTPUT);
}

void loop() {
  if (millis() - lastSend >= 500) {
    lastSend = millis();

    int val = analogRead(LIGHT_PIN);

    // 어두울수록 LED 밝게
    int brightness = map(val, 0, 500, 0, 255);
    brightness = constrain(brightness, 0, 255);

    analogWrite(DIP_R, brightness);
    analogWrite(DIP_G, brightness);
    analogWrite(DIP_B, brightness);

    bleuart.print("light: ");
    bleuart.print(val);
    bleuart.print(" / brightness: ");
    bleuart.println(brightness);
  }
}