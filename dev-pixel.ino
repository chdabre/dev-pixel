// DEV_PIXEL LED Matrix Webserver Display 2018 by Dario Breitenstein.

// LED Matrix Libraries
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Max72xxPanel.h>
// WiFi Libraries
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <WiFiManager.h>
// WebServer Libraries
#include <ESP8266WebServer.h>

// Initialize Web Server on port 80
ESP8266WebServer server(80);

// Initialize LED Matrix
int matrixCS = 2; // The Chip Select pin
Max72xxPanel matrix = Max72xxPanel(matrixCS, 1, 1);

// Display Mode
// 0 = Display IP address
// 1 = Display static image
// 2 = Display animation
// 3 = Display message
int displayMode = 0;

// Message display variables
String message = ""; // Stores the message to display
int msgSpeed = 60; // Delay between moving the message 1 pixel in ms
int msgPos = 0;  // Position of the message relative to the starting position
boolean msgRepeat = false; // Decides if the message should be displayed once or looping

int spacer = 2; // Space between letterPoss
int width = 5 + spacer; // The font width is 5 pixels, so one letterPos takes 5px + the spacers width

// Animation variables
String animFrames = ""; // A chain of hex-encoded images stored in a String
int animLength = 0; // The number of frames stored in animFrames
int animSpeed = 0; // Delay between frames in ms
int animPos = 0; // The index of the current frame
boolean animRepeat = false; // Decides if the animation should be displayed once or looping

String lastImage = "0000000000000000"; // the previously displayed image as hex string

// Draws a binary image encoded as a prefix-free 16-digit hex string to the display
void drawImage(String hex) {
  Serial.println("DRAW IMAGE: " + hex);

  if (hex.length() == 16) {
    for (int i = 0; i < 8; i++) {
      String nextChunk = hex.substring(0, 2); // Get two characters from the string (1 byte)
      char arr[3]; // Null-terminated strings need one extra byte.
      nextChunk.toCharArray(arr, sizeof(arr)); // Convert the string to a char array

      byte thisChunk = strtol(arr, 0, 16); // Convert the hex string to a byte, wich represents one line of pixels

      for (int j = 0; j < 8; j++) {
        matrix.drawPixel(j, i, bitRead(thisChunk, 7 - j)); // Draw the pixel to the frame buffer
      }

      hex = hex.substring(2, hex.length()); // Remove the first two characters from the string to get ready for drawing the next line.
    }
  }

  matrix.write(); // Write the framebuffer to the screen

  lastImage = hex;
}

// Handles incoming HTTP Requests
void handleRequest() {
  Serial.println("HANDLE REQUEST");
  // Debug Arguments
  for (int i = 0; i < server.args(); i++) {
    Serial.println("ARG: " + server.argName(i) + " = " + server.arg(i));
  }

  if (server.args() == 0) {
    Serial.println("ERR: No Arguments provided.");
    server.send(400, "text/plain", "ERR: No Arguments provided"); // Error message
  }
  
  String response = "";

  if (server.arg("intensity") != "") { // Intensity parameter
    int targetIntensity = server.arg("intensity").toInt();

    if (targetIntensity >= 0 && targetIntensity < 16) {
      matrix.setIntensity(targetIntensity); // Set brightness to target Intensity  

      Serial.println("OK: Set Intensity to " + String(targetIntensity));
      response += "OK: Set Intensity to " + String(targetIntensity) + "\n";
    }
    else{

      Serial.println("ERR: The target Intensity must be between 0-15");
      server.send(400, "text/plain", "ERR: The target Intensity must be between 0-15"); // Error message
    } 
  }
  if (server.arg("clear") != "") { // Clear parameter
    displayMode = 1;
    drawImage("0000000000000000");

    Serial.println("OK: Clear Display");
    response += "OK: Clear Display\n";
  }

  // These commands are mutually exclusive and will be interpreted in this order. If another command is added, it will be ignored.
  
  if (server.arg("message") != "") { // Message command
    if (server.arg("repeat") != "") { // Additional repeat argument
      Serial.println("OK: Repeat");
      msgRepeat = true;
    } else {
      msgRepeat = false;
    }
    
    message = server.arg("message");
    msgPos = 0;

    Serial.println("OK: Display Message " + message);
    response += "OK: Display Message " + message + "\n";
  }

  else if (server.arg("animation") != "") { // Animation command
    String argFrames = server.arg("animation");
    String argSpeed = server.arg("speed");
    
    if ( argSpeed == "") {
      Serial.println("ERR: The argument speed is required");
      server.send(400, "text/plain", "ERR: The argument speed is required"); // Error message
    }
    else if (argSpeed.toInt() < 10) {
      Serial.println("ERR: The minimum delay between frames is 10ms");
      server.send(400, "text/plain", "ERR: The minimum delay between frames is 10ms"); // Error message
    }
    else if (argFrames.length() % 16 != 0) {
      Serial.println("ERR: Animation Frames must be divisible by 16, Frame length: " + String(argFrames.length()));
      server.send(400, "text/plain", "ERR: Animation Frames must be divisible by 16"); // Error message
    }
    else {
      if ( server.arg("repeat") != "") { // Additional repeat argument
        Serial.println("OK: Repeat");
        animRepeat = true;
      } else {
        animRepeat = false;
      }

      displayMode = 2;
      animFrames = argFrames;
      animLength = argFrames.length() / 16;
      animSpeed = argSpeed.toInt();
      animPos = 0;

      Serial.println("OK: Display Animation with " + String(animLength) + " Frames");
      response += "OK: Display Animation with " + String(animLength) + " Frames\n";
    }
  }

  else if (server.arg("data") != "") { // Static Image command
    String argImage = server.arg("data");

    if(argImage.length() !=  16){
      Serial.println("ERR: The image must be exactly 16 hex chars");
      server.send(400, "text/plain", "ERR: The image must be exactly 16 hex chars"); // Error message
    }
  
    displayMode = 1;
    drawImage(argImage);

    Serial.println("OK: Display Image " + argImage);
    response = "OK: Display Image " + argImage + "\n";
  }

  if (response.length() > 0){
    Serial.println("RESPONSE: " + response);
    server.send(200, "text/plain", response); //Return the HTTP response
  } else {
    server.send(500, "text/plain", "ERR: Unknown Error"); //Return the HTTP response
  }
  
}

void setup() {
  Serial.begin(115200); // Initialize Serial port

  // Initialize LED matrix
  matrix.setIntensity(8); // Set brightness between 0 and 15
  matrix.setRotation(0, 1); // Set the display rotation to 90 degrees
  matrix.fillScreen(LOW); // Clear the screen
  matrix.write();
  Serial.println("MATRIX OK");
  
  // Initialize WiFi Connection
  WiFiManager wifiManager;
  wifiManager.autoConnect("DevPixel");
  message = WiFi.localIP().toString();
  Serial.println("WIFI OK: "+ WiFi.localIP().toString());
  
  // Initialize WebServer
  server.on("/", handleRequest);
  server.begin();
  Serial.println("HTTP OK");

  Serial.println("READY\n");
}

void loop() {
  server.handleClient(); // Handle incoming requests
  
  if (displayMode == 0 || displayMode == 3) { // Message Display
    if (millis() % msgSpeed == 0) {
      if (msgPos > width * message.length() + matrix.width() - 1 - spacer) { // Message has made 1 pass across the display
        if (!msgRepeat) {
          drawImage(lastImage);
          displayMode = 1; // Static Image mode
        }

        msgPos = 0;

        if (millis() > 25000 && displayMode == 0) {
          msgRepeat = false;
        }
      }

      int letterPos = msgPos / width;
      int x = (matrix.width() - 1) - msgPos % width;
      int y = (matrix.height() - 8) / 2; // center the text vertically

      while ( x + width - spacer >= 0 && letterPos >= 0 ) { // Draw the visible text
        if ( letterPos < message.length() ) {
          matrix.drawChar(x, y, message[letterPos], HIGH, LOW, 1);
        }

        letterPos--;
        x -= width;
      }

      matrix.write(); // Write the frame buffer to the display

      msgPos++;
      delay(1); // Prevent multiple loops within a single ms.
    }
  }
  else if ( displayMode == 2 ) { // Animation Display
    if (millis() % animSpeed == 0) {
      Serial.println("ANIMATION FRAME: " + String(animPos) + "/" + String(animLength));
      if (animPos == animLength) { // End of animation
        if (!animRepeat) {
          matrix.fillScreen(LOW); // Clear the screen
          matrix.write();

          displayMode = 1; // Static Image mode
        } else {
          animPos = 0;
        }
      }

      String currentFrame = animFrames.substring(animPos * 16, animPos * 16 + 16);
      drawImage(currentFrame); // Draw the image at the current position

      animPos++;
      delay(1); // Prevent multiple loops within a single ms.
    }
  }

}


