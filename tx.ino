#include <RadioLib.h>

// SX1262 pin definitions
#define NSS_PIN 41    // NSS/CS pin
#define DIO1_PIN 39   // DIO1 pin
#define RESET_PIN 42  // RESET pin
#define BUSY_PIN 40   // BUSY pin

// Create SX1262 instance
SX1262 radio = new Module(NSS_PIN, DIO1_PIN, RESET_PIN, BUSY_PIN);

unsigned long timeout = 5000; // 5-second timeout for acknowledgement

void setup() {
  Serial.begin(115200);
  Serial.println(F("Setting up sender node..."));

  // Initialize the SX1262 radio
  int state = radio.begin(868.0);  // Frequency in MHz (adjust for your region)
  if (state == RADIOLIB_ERR_NONE) {
    Serial.println(F("SX1262 initialized successfully"));
    radio.setSpreadingFactor(7);  // Spreading Factor
    radio.setBandwidth(125.0);    // Bandwidth in kHz
    radio.setCodingRate(5);       // Coding Rate (4/5)
  } else {
    Serial.print(F("SX1262 initialization failed, code "));
    Serial.println(state);
    while (true);
  }
}

void loop() {
  const char* message = "Hello, SX1262!";
  Serial.print(F("Sending message: "));
  Serial.println(message);

  // Send message and wait for acknowledgement
  int state = radio.transmit(message);
  if (state == RADIOLIB_ERR_NONE) {
    // Serial.println(F("Message sent successfully"));
    if (waitForAck()) {
      Serial.println(F("Acknowledgement received - confirming"));
      radio.transmit("ack");
    } else {
      Serial.println(F("Acknowledgement not received. Retrying..."));
      delay(5000); // Retry delay
    }
  } else {
    Serial.print(F("Message send failed, code "));
    Serial.println(state);
  }

  delay(6000);  // Wait before sending the next message
}

bool waitForAck() {
  Serial.println(F("Waiting for acknowledgement..."));
  unsigned long startTime = millis();
  String message;

  while (millis() - startTime < timeout) {
    int state = radio.receive(message);
    if (state == RADIOLIB_ERR_NONE) {
      // Serial.print(F("Received acknowledgement: "));
      // Serial.println(message);
      return (message == "ack");  // Return true if acknowledgement matches
    }
  }

  Serial.println(F("Timeout waiting for acknowledgement."));
  return false; // No acknowledgement received
}
