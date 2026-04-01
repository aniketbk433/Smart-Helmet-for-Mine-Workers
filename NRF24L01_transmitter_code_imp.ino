RF24 by TMRh20
#include <SPI.h>
#include <RF24.h>

#define CE_PIN 17
#define CSN_PIN 5

RF24 radio(CE_PIN, CSN_PIN);
const byte address[6] = "00001";

unsigned long packetCounter = 0;
const unsigned long sendInterval = 200; // 200 ms between packets for calibration

void setup() {
  Serial.begin(115200);
  delay(1000);

  if (!radio.begin()) {
    Serial.println("ERROR: nRF24L01 not detected!");
    while (1);
  }

  radio.setPALevel(RF24_PA_HIGH);
  radio.setDataRate(RF24_1MBPS);
  radio.setChannel(76);
  radio.openWritingPipe(address);
  radio.stopListening();

  Serial.println("ESP32 Transmitter Initialized");
}

void loop() {
  struct Packet {
    unsigned long counter;
    unsigned long timestamp;
  } data;

  data.counter = ++packetCounter;
  data.timestamp = millis();

  bool success = radio.write(&data, sizeof(data));

  if (success) Serial.println("Packet sent: " + String(packetCounter));
  else Serial.println("Send failed!");

  delay(sendInterval);
}
