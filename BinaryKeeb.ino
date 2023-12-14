// Bluetooth Binary Keyboard 
// Dan Smalley 2023 
// Version 1.0
// Inspired by this project by Ryan Wise https://ryanwise.me/projects/binary-keyboard/ 

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <BleKeyboard.h>

// Initialize Screen
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Initialize Bluetooth
BleKeyboard bleKeyboard("BinaryKeeb", "DS", 100);
#define USE_NIMBLE;

// Initialize Pins
const int ZERO_PIN = D0;
const int ONE_PIN = D1;
const int TWO_PIN = D2;
const int THREE_PIN = D3;

// Can't use interrupts because things like SPI for the display won't work.
bool zeroPressed = true;
bool onePressed = true;

// Initialize bit stack 
unsigned int currentByte = 0;
unsigned int count = 0;

String debugData = "";

// Main setup block
void setup() {
  Serial.begin(9600);

  pinMode(ZERO_PIN, INPUT_PULLUP);
  pinMode(ONE_PIN, INPUT_PULLUP);
  pinMode(TWO_PIN, INPUT_PULLUP);

  // Start bluetooth service
  bleKeyboard.begin();


  // Generate display voltage from 3.3 internally - from SSD1306 example
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("Display failed to load"));
    for (;;); // go into permanenet loop so we don't break stuff 
  }

  // Display welcome info
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);
  display.println("BLE Binary Keyboard");
  display.setCursor(0,10);
  display.println("Dan Smalley 2023");
  display.setCursor(0,20);
  display.println("Version 1.0");
  display.display();

  sleep(5);

  // Draw initial text
  drawBits();
}


void loop() {
  delay(10);

  // Check if a key is pressed and add bits accordingly 
  if (clicked(&zeroPressed, ZERO_PIN)) {
    addBit(0);
  }
  if (clicked(&onePressed, ONE_PIN)) {
    addBit(1);
  }
}


bool clicked(bool *isPressed, int pin) {
  // Reads key state from digital IO pins 
  if (digitalRead(pin) == HIGH && !(*isPressed)) {
    *isPressed = true;
    return true;
  } else if (digitalRead(pin) == LOW) {
    *isPressed = false;
  }

  return false;
}


void addBit(int b) {
  // Adds our bits to the stack, credit for this method 100% goes to Ryan
  // Shift bits left
  currentByte <<= 1;
  // Modify least significant bit
  currentByte |= b;
  
  count++;

  drawBits();

  if (count == 8) {
    // Once we have a full byte of bits output the value to the BT keyboard interface
    serialDebug(currentByte);
    bleKeyboard.write((int)currentByte);
    // Leave the last drawn bit on the screen for half a second to make it more visually pleasing
    sleep(0.5);
    // Draw the ASCII character represented by the byte value on screen 
    display.clearDisplay();
    display.setCursor(60,8);
    display.println((char)currentByte);
    display.display();
    // Reset counters 
    currentByte = 0;
    count = 0;
  }
}

void serialDebug (int cb) {
    Serial.print((int)cb);
    Serial.print("\t");
    Serial.print((char)cb);
    Serial.println();
}

void drawBits() {
  // Again, credit to Ryan for this method 
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,8);
  // Draws a dash followed by the current bit stack and underscores for all remaining bits followed by another dash 
  // ie. -100011__-
  String output = "-";

  for (int i = count; i > 0; i--) {
    output += ((currentByte >> i-1) & 1) == 0 ? "0" : "1";
  }

  for (int i = count; i <= 7; i++) {
    output += "_";
  }

  output += "-";

  display.println(output);
  display.display();
}