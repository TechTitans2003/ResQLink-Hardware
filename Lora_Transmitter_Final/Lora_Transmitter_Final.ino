#include <SPI.h>
#include <LoRa.h>

//======================================================================//

#define PIN_LORA_COPI   13
#define PIN_LORA_CIPO   12
#define PIN_LORA_SCK    14
#define PIN_LORA_CS     15
#define PIN_LORA_RST    16
#define PIN_LORA_DIO0   5

#define LORA_FREQUENCY  433E6
#define TRANSMITTER_ID  240906  // Unique ID for each transmitter

//======================================================================//

void setup() {
  Serial.begin(115200);
  while (!Serial);
  delay(1500);
  Serial.println("LoRa Transmitter");

  LoRa.setPins(PIN_LORA_CS, PIN_LORA_RST, PIN_LORA_DIO0);
  LoRa.setSPIFrequency(20000000);

  if (!LoRa.begin(LORA_FREQUENCY)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  } else {
    Serial.print("LoRa initialized with frequency ");
    Serial.println(LORA_FREQUENCY);
  }
}

//======================================================================//

void loop() {
  String message = "Signal Sent 3!";
  String transmitterIDStr = String(TRANSMITTER_ID);  // Convert TRANSMITTER_ID to a string

  Serial.print("Sending: ");
  Serial.println(transmitterIDStr + " - " + message);

  // Begin sending the packet
  LoRa.beginPacket();
  LoRa.print(transmitterIDStr);  // Send the transmitter ID as part of the message
  LoRa.print(" - ");
  LoRa.print(message);  // Send the message
  LoRa.endPacket();

  delay(3000);  // Delay between messages
}


//======================================================================//
