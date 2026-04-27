#include <Adafruit_GFX.h>
#include <Adafruit_TFTLCD.h>
#include <TouchScreen.h>
#include "HX711.h"  // Load cell amplifier

// Color definitions
#define BLACK   0x0000
#define WHITE   0xFFFF
#define GREEN   0x07E0
#define RED     0xF800
#define BLUE    0x001F

// TFT Display pins
#define LCD_CS A3
#define LCD_CD A2
#define LCD_WR A1
#define LCD_RD A0
#define LCD_RESET A4

// Touchscreen pins
#define YP A2  
#define XM A3  
#define YM 8   
#define XP 9   

// Load cell pins
#define LOADCELL_DOUT 10  
#define LOADCELL_SCK  12  

// Button properties
#define BUTTON_WIDTH 160
#define BUTTON_HEIGHT 40
#define BUTTON_X ((320 - BUTTON_WIDTH) / 2)  

#define ZERO_BUTTON_Y 0    
#define RESET_BUTTON_Y 200 

#define TOUCH_PADDING 20  // Increase touch sensitivity

// Terminal Line Properties
#define TERMINAL_X 40  // Align to "Force" text start
#define TERMINAL_Y 180 // Keep it in line with force/max labels
#define TERMINAL_WIDTH 250  // Enough width to clear previous text
#define TERMINAL_HEIGHT 15  // Small space for the message

// Initialize components
Adafruit_TFTLCD tft(LCD_CS, LCD_CD, LCD_WR, LCD_RD, LCD_RESET);
TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);
HX711 scale;  

float calibration_factor = 7050;  
String lastMessage = "";  // Stores the last message displayed

void drawZeroButton() {
    tft.fillRect(BUTTON_X, ZERO_BUTTON_Y, BUTTON_WIDTH, BUTTON_HEIGHT, RED);
    tft.setCursor(BUTTON_X + 35, ZERO_BUTTON_Y + 10);
    tft.setTextColor(WHITE, RED);
    tft.setTextSize(2);
    tft.print("ZERO");
}

void drawResetButton() {
    tft.fillRect(BUTTON_X, RESET_BUTTON_Y, BUTTON_WIDTH, BUTTON_HEIGHT, BLUE);
    tft.setCursor(BUTTON_X + 35, RESET_BUTTON_Y + 10);
    tft.setTextColor(WHITE, BLUE);
    tft.setTextSize(2);
    tft.print("RESET");
}

void updateTerminalLine(String message) {
    if (message != lastMessage) {  // Only update if message has changed
        Serial.print("Updating terminal: "); Serial.println(message);  // Debugging
        tft.fillRect(TERMINAL_X, TERMINAL_Y, TERMINAL_WIDTH, TERMINAL_HEIGHT, BLACK);  // Clear previous text
        tft.setCursor(TERMINAL_X, TERMINAL_Y);
        tft.setTextColor(WHITE, BLACK);
        tft.setTextSize(1);  // Smaller text
        tft.print(message);
        lastMessage = message;  // Store last message to prevent unnecessary refreshes
    }
}

void setup() {
    Serial.begin(9600);

    // Initialize display
    tft.begin(0x7575);  
    tft.setRotation(1);
    tft.fillScreen(BLACK);
    tft.setTextSize(2);

    // Draw UI
    drawZeroButton();
    drawResetButton();

    // Initialize load cell
    scale.begin(LOADCELL_DOUT, LOADCELL_SCK);
    scale.set_scale(calibration_factor);
    scale.tare();  
    delay(1000);  

    updateTerminalLine("System Ready.");
}

void loop() {
    // Read force sensor value
    float forceValue = scale.get_units(10) * 9.81;  
    static float maxForce = 0;

    if (forceValue > maxForce) {
        maxForce = forceValue;
    }

    // Temporarily disable touch while updating TFT
    pinMode(XM, OUTPUT);
    pinMode(YP, OUTPUT);
    
    // Update force display only if value changes
    static float prevForce = -1, prevMaxForce = -1;
    if (forceValue != prevForce) {
        tft.fillRect(150, 80, 100, 30, BLACK);
        tft.setCursor(40, 80);
        tft.setTextColor(GREEN, BLACK);
        tft.setTextSize(3);
        tft.print("Force (N):");
        tft.setCursor(150, 80);
        tft.print(forceValue, 1);
        tft.print(" N  ");
        prevForce = forceValue;
    }

    if (maxForce != prevMaxForce) {
        tft.fillRect(150, 130, 100, 30, BLACK);
        tft.setCursor(40, 130);
        tft.setTextColor(RED, BLACK);
        tft.setTextSize(3);
        tft.print("Max:");
        tft.setCursor(150, 130);
        tft.print(maxForce, 1);
        tft.print(" N  ");
        prevMaxForce = maxForce;
    }

    // Re-enable touch input
    pinMode(XM, INPUT);
    pinMode(YP, INPUT);

    // Detect touchscreen input
    TSPoint p = ts.getPoint();
    if (p.z > 10 && p.z < 1000) {
        int screenX = map(p.x, 142, 907, 320, 0);
        int screenY = map(p.y, 158, 872, 240, 0);

        updateTerminalLine("Touch: X=" + String(screenX) + " Y=" + String(screenY));

        // Zero button
        if (screenX > BUTTON_X - TOUCH_PADDING && screenX < BUTTON_X + BUTTON_WIDTH + TOUCH_PADDING &&
            screenY > ZERO_BUTTON_Y - TOUCH_PADDING && screenY < ZERO_BUTTON_Y + BUTTON_HEIGHT + TOUCH_PADDING) {
            
            Serial.println("Zero button pressed!");  // Debugging
            updateTerminalLine("Zero button pressed!");
            scale.tare();  
            forceValue = 0;
            prevForce = -1;  

            tft.fillRect(150, 80, 100, 30, BLACK);
            tft.setCursor(40, 80);
            tft.setTextColor(GREEN, BLACK);
            tft.setTextSize(3);
            tft.print("Force (N):");
            tft.setCursor(150, 80);
            tft.print("0.0 N  ");
        }

        // Reset button
        if (screenX > BUTTON_X - TOUCH_PADDING && screenX < BUTTON_X + BUTTON_WIDTH + TOUCH_PADDING &&
            screenY > RESET_BUTTON_Y - TOUCH_PADDING && screenY < RESET_BUTTON_Y + BUTTON_HEIGHT + TOUCH_PADDING) {
            
            Serial.println("Reset button pressed!");  // Debugging
            updateTerminalLine("Reset button pressed!");
            maxForce = 0;
            prevMaxForce = -1;  

            tft.fillRect(150, 130, 100, 30, BLACK);
            tft.setCursor(40, 130);
            tft.setTextColor(RED, BLACK);
            tft.setTextSize(3);
            tft.print("Max:");
            tft.setCursor(150, 130);
            tft.print("0.0 N  ");
        }
    }

    delay(100);
}
