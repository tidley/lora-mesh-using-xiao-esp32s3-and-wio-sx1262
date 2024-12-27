#include <Arduino.h>
#include <U8x8lib.h>
#include <Wire.h>
#include <RadioLib.h>

// SX1262 pin definitions
#define NSS_PIN 41   // NSS/CS pin
#define DIO1_PIN 39  // DIO1 pin
#define RESET_PIN 42 // RESET pin
#define BUSY_PIN 40  // BUSY pin

// Screen setup
U8X8_SSD1306_128X64_NONAME_HW_I2C u8x8(/* clock=*/SCL, /* data=*/SDA, /* reset=*/U8X8_PIN_NONE); // OLEDs without Reset of the Display

// Create SX1262 instance
SX1262 radio = new Module(NSS_PIN, DIO1_PIN, RESET_PIN, BUSY_PIN);

unsigned long startTime = 0;
unsigned long timeout = 2000; // 2-second timeout for acknowledgement

// int messageCount = 0;

void setup()
{
    // Screem
    u8x8.begin();
    u8x8.setFlipMode(1); // set number from 1 to 3, the screen word will rotary 180
    u8x8.setFont(u8x8_font_chroma48medium8_r);

    Serial.begin(115200);

    serialPlusScreen("Setting up receiver node...");

    // Initialize the SX1262 radio
    int state = radio.begin(868.0); // Frequency in MHz (adjust for your region)
    if (state == RADIOLIB_ERR_NONE)
    {
        String dbgMSg = "SX1262 initialized successfully";
        serialPlusScreen(dbgMSg);

        radio.setSpreadingFactor(7); // Spreading Factor
        radio.setBandwidth(125.0);   // Bandwidth in kHz
        radio.setCodingRate(5);      // Coding Rate (4/5)
    }
    else
    {
        String dbgMSg = "SX1262 initialization failed, code " + String(state);
        serialPlusScreen(dbgMSg);
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
            serialPlusScreen("Received ack");
        }
        else
        {
            String dbgMSg = "Received: " + String(receivedMessage);
            serialPlusScreen(dbgMSg);
            // delay(1000);

            ack("ack");
        }
    }
}

void ack(const char *message)
{

    // String dbgMSg = "Sending: " + String(message);
    // serialPlusScreen(dbgMSg);

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
                // serialPlusScreen("Acknowledgement confirmed");
                delay(500);

                return; // Exit after successful acknowledgement exchange
            }
            else
            {
                serialPlusScreen("No confirmation, retrying...");
            }
        }
        else
        {
            String dbgMSg = "Failed to send acknowledgement, code: " + String(state);
            serialPlusScreen(dbgMSg);
        }
        delay(500); // Delay before retrying
    }
    serialPlusScreen("Failed to confirm acknowledgement after retries.");
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
        String dbgMSg = "Receive error, code: " + String(state);
        serialPlusScreen(dbgMSg);
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
    serialPlusScreen("Timeout waiting for acknowledgement");
    return ""; // Return empty string if timeout occurs
}

void serialPlusScreen(String message)
{
    Serial.println(message);
    writeToScreen(message);
}

void writeToScreen(String message)
{
    u8x8.clear();
    u8x8.setCursor(0, 0);
    for (int i = 0; i < message.length(); i += 15)
    {
        u8x8.print(message.substring(i, i + 15));
        u8x8.setCursor(0, (i / 15) + 1);
    }
}