#include <WiFi.h>
#include <TFT_eSPI.h> // Include the TFT_eSPI library

// Initialize TFT display
TFT_eSPI tft = TFT_eSPI();

// Define IR sensor pins
#define IR_SENSOR1_PIN 12
#define IR_SENSOR2_PIN 13
#define IR_SENSOR3_PIN 17
#define IR_SENSOR4_PIN 18

// Wi-Fi credentials
const char *ssid = "Your_SSID";
const char *password = "Your_PASSWORD";

// Create a WiFi server
WiFiServer server(80);

void setup() {
  // Initialize serial communication
  Serial.begin(115200);

  // Set IR sensor pins as input
  pinMode(IR_SENSOR1_PIN, INPUT);
  pinMode(IR_SENSOR2_PIN, INPUT);
  pinMode(IR_SENSOR3_PIN, INPUT);
  pinMode(IR_SENSOR4_PIN, INPUT);

  // Initialize the display
  tft.init();
  tft.setRotation(1); // Adjust orientation if needed
  tft.fillScreen(TFT_BLACK); // Clear the screen
  tft.setTextColor(TFT_WHITE, TFT_BLACK); // White text, black background
  tft.setTextSize(2); // Set text size
  tft.drawString("KARE Parking", 10, 10);

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected.");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // Start the server
  server.begin();
}

void loop() {
  // Update TFT Display
  displayDetectedSensors();

  // Handle web server
  WiFiClient client = server.available();
  if (client) {
    String request = client.readStringUntil('\r');
    client.flush();

    bool detected1 = digitalRead(IR_SENSOR1_PIN) == LOW;
    bool detected2 = digitalRead(IR_SENSOR2_PIN) == LOW;
    bool detected3 = digitalRead(IR_SENSOR3_PIN) == LOW;
    bool detected4 = digitalRead(IR_SENSOR4_PIN) == LOW;

    if (request.indexOf("/data") >= 0) {
      // Serve dynamic sensor data
      client.println("HTTP/1.1 200 OK");
      client.println("Content-type:text/plain");
      client.println("Connection: close");
      client.println();
      client.println(generateData(detected1, detected2, detected3, detected4));
    } else {
      // Serve the main HTML page
      client.println("HTTP/1.1 200 OK");
      client.println("Content-type:text/html");
      client.println("Connection: close");
      client.println();
      client.println(generateHTML(detected1, detected2, detected3, detected4));
    }
    client.stop();
  }

  delay(100); // Small delay for responsiveness
}

// Function to display detected sensors on the TFT
void displayDetectedSensors() {
  tft.fillScreen(TFT_BLACK); // Clear the screen

  if (digitalRead(IR_SENSOR1_PIN) == LOW &&
      digitalRead(IR_SENSOR2_PIN) == LOW &&
      digitalRead(IR_SENSOR3_PIN) == LOW &&
      digitalRead(IR_SENSOR4_PIN) == LOW) {
    tft.setTextColor(TFT_RED, TFT_BLACK);
    tft.drawString("Parking Full!", 10, 10);
    return;
  }

  // Show name or other information at the top
  displayName();

  int yPos = 40; // Starting position for sensor messages
  if (digitalRead(IR_SENSOR1_PIN) == LOW) {
    tft.drawString("Slot 1 Parked!", 10, yPos);
    yPos += 20;
  }
  if (digitalRead(IR_SENSOR2_PIN) == LOW) {
    tft.drawString("Slot 2 Parked!", 10, yPos);
    yPos += 20;
  }
  if (digitalRead(IR_SENSOR3_PIN) == LOW) {
    tft.drawString("Slot 3 Parked!", 10, yPos);
    yPos += 20;
  }
  if (digitalRead(IR_SENSOR4_PIN) == LOW) {
    tft.drawString("Slot 4 Parked!", 10, yPos);
    yPos += 20;
  }

  if (yPos == 40) {
    tft.drawString("No Vehicle Parked", 10, yPos);
  }
}

// Function to display the name or header
void displayName() {
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.setTextSize(1);
  tft.drawString("Parking Available", 5, 5);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(2);
}
// Function to generate the main HTML page
String generateHTML(bool detected1, bool detected2, bool detected3, bool detected4) {
  String html = "<!DOCTYPE html><html><head><title>KARE Parking</title>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
  html += "<link rel='stylesheet' href='https://cdnjs.cloudflare.com/ajax/libs/font-awesome/5.15.4/css/all.min.css'>"; // Font Awesome CSS
  html += "<script>";
  html += "let lastState = ''; function updatePage(data) { const container = document.getElementById('content'); if (data !== lastState) { container.innerHTML = data; lastState = data; } }";
  html += "function fetchData() { fetch('/data').then(response => response.text()).then(updatePage).catch(() => {}); }";
  html += "setInterval(fetchData, 1000);";
  html += "</script>";
  html += "<style>";
  html += "body { font-family: Arial; text-align: center; background: lightblue; margin: 0; padding: 0; }";
  
  // Styling for the messages
  html += ".status { font-size: 36px; font-weight: bold; padding: 20px; margin: 20px; border-radius: 10px; color: white; }";
  html += ".available { background-color: green; }";
  html += ".full { background-color: red; }";

  // Styling for the circles
  html += ".circle-container { display: flex; justify-content: center; gap: 20px; flex-wrap: wrap; margin-top: 40px; }";
  html += ".circle { width: 100px; height: 100px; border-radius: 50%; display: flex; align-items: center; justify-content: center; font-size: 30px; color: white; position: relative; }";
  html += ".green { background-color: green; } .blue { background-color: blue; } .red { background-color: red; } .purple { background-color: purple; }";
  
  // Styling for status below circles
  html += ".slot-status { font-size: 18px; color: black; margin-top: 10px; font-weight: bold; }";
  html += ".fa-car-side { font-size: 48px; }";
  
  html += "</style></head><body>";
  html += "<div id='content'></div></body></html>";
  return html;
}

// Function to generate dynamic sensor data with slot info
String generateData(bool detected1, bool detected2, bool detected3, bool detected4) {
  String data = "<div>";

  // Display Parking status
  if (detected1 && detected2 && detected3 && detected4) {
    data += "<div class='status full'>Parking Full</div>";
  } else {
    data += "<div class='status available'>Parking Available</div>";
  }

  // Circle container with the sensors
  data += "<div class='circle-container'>";
  
  // Slot 1 - Green Circle
  if (detected1) {
    data += "<div class='circle green'><i class='fas fa-car-side'></i></div>";
    data += "<div class='slot-status'>Slot 1 Parked</div>";
  } else {
    data += "<div class='circle green'></div>";
    data += "<div class='slot-status'>Slot 1 Available</div>";
  }

  // Slot 2 - Blue Circle
  if (detected2) {
    data += "<div class='circle blue'><i class='fas fa-car-side'></i></div>";
    data += "<div class='slot-status'>Slot 2 Parked</div>";
  } else {
    data += "<div class='circle blue'></div>";
    data += "<div class='slot-status'>Slot 2 Available</div>";
  }

  // Slot 3 - Red Circle
  if (detected3) {
    data += "<div class='circle red'><i class='fas fa-car-side'></i></div>";
    data += "<div class='slot-status'>Slot 3 Parked</div>";
  } else {
    data += "<div class='circle red'></div>";
    data += "<div class='slot-status'>Slot 3 Available</div>";
  }

  // Slot 4 - Purple Circle
  if (detected4) {
    data += "<div class='circle purple'><i class='fas fa-car-side'></i></div>";
    data += "<div class='slot-status'>Slot 4 Parked</div>";
  } else {
    data += "<div class='circle purple'></div>";
    data += "<div class='slot-status'>Slot 4 Available</div>";
  }

  data += "</div>"; // End of circle-container
  data += "</div>";
  return data;
}