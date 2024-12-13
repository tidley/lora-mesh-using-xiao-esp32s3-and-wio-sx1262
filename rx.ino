#include <RadioLib.h>

// SX1262 pin definitions
#define NSS_PIN 41   // NSS/CS pin
#define DIO1_PIN 39  // DIO1 pin
#define RESET_PIN 42 // RESET pin
#define BUSY_PIN 40  // BUSY pin

// Create SX1262 instance
SX1262 radio = new Module(NSS_PIN, DIO1_PIN, RESET_PIN, BUSY_PIN);

unsigned long startTime = 0;
unsigned long timeout = 2000; // 2-second timeout for acknowledgement

void setup()
{
    Serial.begin(115200);
    Serial.println(F("Setting up node..."));

    // Initialize the SX1262 radio
    int state = radio.begin(868.0); // Frequency in MHz (adjust for your region)
    if (state == RADIOLIB_ERR_NONE)
    {
        Serial.println(F("SX1262 initialized successfully"));
        radio.setSpreadingFactor(7); // Spreading Factor
        radio.setBandwidth(125.0);   // Bandwidth in kHz
        radio.setCodingRate(5);      // Coding Rate (4/5)
    }
    else
    {
        Serial.print(F("SX1262 initialization failed, code "));
        Serial.println(state);
        while (true)
            ;
    }
}

void loop()
{
    // Receive a message and respond with an acknowledgement
    String receivedMessage = rx();
    if (receivedMessage != "")
    {
        if (receivedMessage == "ack")
        {
            Serial.println("Received ack");
        }
        else
        {
            Serial.print(F("Received message: "));
            Serial.println(receivedMessage);
            ack("ack");
        }
    }
}

void ack(const char *message)
{
    Serial.print(F("Sending: "));
    Serial.println(message);

    int retries = 3; // Retry acknowledgement 3 times
    while (retries--)
    {
        int state = radio.transmit(message);
        if (state == RADIOLIB_ERR_NONE)
        {
            // Serial.println(F("Acknowledgement sent successfully"));

            // Wait for acknowledgement of acknowledgement
            String ackResponse = rxWithTimeout();
            if (ackResponse == "ack")
            {
                Serial.println(F("Acknowledgement confirmed"));
                return; // Exit after successful acknowledgement exchange
            }
            else
            {
                Serial.println(F("No confirmation, retrying..."));
            }
        }
        else
        {
            Serial.print(F("Failed to send acknowledgement, code "));
            Serial.println(state);
        }
        delay(500); // Delay before retrying
    }
    Serial.println(F("Failed to confirm acknowledgement after retries."));
}

String rx()
{
    String message;
    int state = radio.receive(message);
    if (state == RADIOLIB_ERR_NONE)
    {
        return message; // Return the received message
    }
    else if (state == RADIOLIB_ERR_RX_TIMEOUT)
    {
        // Serial.println(F("Receive timeout, no message received."));
    }
    else
    {
        Serial.print(F("Receive error, code "));
        Serial.println(state);
    }
    return ""; // Return empty string if no message received
}

String rxWithTimeout()
{
    String message;
    startTime = millis();
    while (millis() - startTime < timeout)
    {
        int state = radio.receive(message);
        if (state == RADIOLIB_ERR_NONE)
        {
            // Serial.print(F("Received message: "));
            // Serial.println(message);
            return message;
        }
    }
    Serial.println(F("Timeout waiting for acknowledgement"));
    return ""; // Return empty string if timeout occurs
}
