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

unsigned long loopTimer = 1000;
unsigned long timeout = 5000; // timeout for acknowledgement

int messageCount = 0;

void setup()
{
    // Screen
    u8x8.begin();
    u8x8.setFlipMode(1); // set number from 1 to 3, the screen word will rotary 180
    u8x8.setFont(u8x8_font_chroma48medium8_r);

    // Radio
    Serial.begin(115200);
    // Serial.println(F("Setting up sender node..."));

    serialPlusScreen("Setting up sender node...");

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
    String message = "Hello times " + String(messageCount) + "!";

    String dbgMSg = "Sending message: " + String(message);
    serialPlusScreen(dbgMSg);

    // Send message and wait for acknowledgement
    int state = radio.transmit(message);
    if (state == RADIOLIB_ERR_NONE)
    {
        // Serial.println(F("Message sent successfully"));
        if (waitForAck())
        {
            serialPlusScreen("Acknowledgement " + String(messageCount) + " received - confirming");
            radio.transmit("ack");
            messageCount++;
        }
        else
        {
            serialPlusScreen("Acknowledgement not received. Retrying in 2 seconds...");
            delay(2000);
        }
    }
    else
    {
        serialPlusScreen("Message send failed, code " + String(state));
    }

    delay(loopTimer); // Wait before sending the next message
}

bool waitForAck()
{
    serialPlusScreen("Waiting for acknowledgement...");
    unsigned long startTime = millis();
    String message;

    while (millis() - startTime < timeout)
    {
        int state = radio.receive(message);
        if (state == RADIOLIB_ERR_NONE)
        {
            // Serial.print(F("Received acknowledgement: "));
            // Serial.println(message);
            return (message == "ack"); // Return true if acknowledgement matches
        }
    }

    serialPlusScreen("Timeout waiting for acknowledgement");
    return false; // No acknowledgement received
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