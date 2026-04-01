//install RF24 by TMRh20
#include <SPI.h>
#include <RF24.h>

#define CE_PIN 4    // D2 = GPIO4
#define CSN_PIN 15  // D8 = GPIO15

RF24 radio(CE_PIN, CSN_PIN);
const byte address[6] = "00001";

unsigned long lastReceivedTime = 0;
const int sampleCount = 10; // number of packets per measurement cycle

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("Calibrated ESP8266 Receiver for 0-500m Starting...");

  if (!radio.begin()) {
    Serial.println("ERROR: nRF24L01 not detected!");
    while (1);
  }

  if (!radio.isChipConnected()) {
    Serial.println("ERROR: nRF24L01 connection failed!");
    while (1);
  }

  radio.setPALevel(RF24_PA_HIGH);     // maximum power
  radio.setDataRate(RF24_1MBPS);      // reliable for long range
  radio.setChannel(76);
  radio.openReadingPipe(0, address);
  radio.startListening();

  Serial.println("Receiver Initialized Successfully");
}

void loop() {
  int rpdCount = 0;
  struct Packet {
    unsigned long counter;
    unsigned long timestamp;
  } data;

  int packetsReceived = 0;

  // Sample multiple packets for calibration
  for (int i = 0; i < sampleCount; i++) {
    unsigned long start = millis();
    while (!radio.available() && millis() - start < 100) {
      // wait max 100ms for packet
    }

    if (radio.available()) {
      radio.read(&data, sizeof(data));
      packetsReceived++;
      lastReceivedTime = millis();

      if (radio.testRPD()) rpdCount++;
    }
  }

  if (packetsReceived > 0) {
    float rpdRatio = (float)rpdCount / packetsReceived;

    // Corrected thresholds for proper near-distance detection
    String distanceCategory = "Very Far";
    if (rpdRatio > 0.95) distanceCategory = "Near";
    else if (rpdRatio > 0.85) distanceCategory = "Very Close";
    else if (rpdRatio > 0.7) distanceCategory = "Close";
    else if (rpdRatio > 0.5) distanceCategory = "Medium";
    else if (rpdRatio > 0.3) distanceCategory = "Far";
    else if (rpdRatio > 0.1) distanceCategory = "Very Far";

    Serial.print("Packet#: "); Serial.print(data.counter);
    Serial.print(" | Packets Received: "); Serial.print(packetsReceived);
    Serial.print(" | RPD Ratio: "); Serial.print(rpdRatio, 2);
    Serial.print(" | Distance: "); Serial.println(distanceCategory);
  }

  // Diagnostic mode if no packets for 5s
  if (millis() - lastReceivedTime > 5000) {
    Serial.println("** No communication detected **");
    lastReceivedTime = millis();
  }
}
