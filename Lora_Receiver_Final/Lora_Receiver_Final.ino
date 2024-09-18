#include <SPI.h>
#include <LoRa.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <ESP8266HTTPClient.h>

// WiFi credentials
const char* ssid = "lora";
const char* password = "lora1234";

// Server URL (use HTTPS)
const char* serverHost = "https://resqlink.onrender.com";
const char* serverPath = "/api/sensor/send";  // Path for POSTing sensor data

// LoRa setup
#define LORA_FREQUENCY 433E6  // Frequency for LoRa

// Pin definitions for LoRa
#define PIN_LORA_COPI   13
#define PIN_LORA_CIPO   12
#define PIN_LORA_SCK    14
#define PIN_LORA_CS     15
#define PIN_LORA_RST    16
#define PIN_LORA_DIO0   5

void setup() {
  Serial.begin(115200);

  // Connect to WiFi
  Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid, password);

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(1000);
    Serial.print(".");
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nConnected to WiFi");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nFailed to connect to WiFi");
    while (true) {
      delay(1000);  // Halt the program if unable to connect
    }
  }

  // Initialize LoRa
  Serial.println("Initializing LoRa...");
  LoRa.setPins(PIN_LORA_CS, PIN_LORA_RST, PIN_LORA_DIO0);  // Set LoRa pins
  LoRa.setSPIFrequency(20000000);  // Set SPI frequency

  if (!LoRa.begin(LORA_FREQUENCY)) {
    Serial.println("LoRa initialization failed!");
    while (true) {
      delay(1000);  // Halt the program if LoRa fails to initialize
    }
  }
  Serial.println("LoRa Initialized");
}

void loop() {
  // Check for incoming LoRa packets
  int packetSize = LoRa.parsePacket();

  if (packetSize) {
    String receivedMessage = "";
    while (LoRa.available()) {
      receivedMessage += (char)LoRa.read();  // Read the packet
    }

    Serial.print("Received: ");
    Serial.println(receivedMessage);

    // Split the message to extract the transmitter ID and the actual message
    int separatorIndex = receivedMessage.indexOf(" - ");
    if (separatorIndex == -1) {
      Serial.println("Invalid LoRa message format");
      return;  // Exit if message is not in expected format
    }

    String transmitterID = receivedMessage.substring(0, separatorIndex);  // Extract the transmitter ID
    String message = receivedMessage.substring(separatorIndex + 3);  // Extract the actual message

    int rssi = LoRa.packetRssi();  // Get the signal strength

    // Prepare JSON data to send
    String jsonData = "{\"name\":\"" + String(transmitterID) + "\",\"rssi\":" + String(rssi) + "}";

    // Check if WiFi is connected
    if (WiFi.status() == WL_CONNECTED) {
      HTTPClient http;
      WiFiClientSecure client;  // Use WiFiClientSecure for HTTPS
      client.setInsecure();     // This disables SSL certificate verification (for testing)

      Serial.println("Connecting to server...");

      // Use serverHost and serverPath to specify HTTPS URL
      String fullURL = String(serverHost) + String(serverPath);
      http.begin(client, fullURL);  // Initialize HTTPS connection
      http.addHeader("Content-Type", "application/json");  // Set content type

      // Send the PATCH request with JSON data
      int httpResponseCode = http.PATCH(jsonData);

      // Print the response code and handle potential errors
      if (httpResponseCode > 0) {
        String response = http.getString();  // Get the response from the server
        Serial.println("HTTP Response code: " + String(httpResponseCode));
        Serial.println("Server response: " + response);
      } else {
        Serial.println("Error on HTTP request. HTTP Response code: " + String(httpResponseCode));
      }

      http.end();  // Close the HTTP connection
    } else {
      Serial.println("WiFi not connected, retrying...");
    }

    delay(1000);  // Wait before sending the next packet
  } else {
    // No LoRa packet available, loop again
    delay(1000);
  }
}
