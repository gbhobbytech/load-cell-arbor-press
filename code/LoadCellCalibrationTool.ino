#include "HX711.h"

#define DOUT 10
#define CLK 12

HX711 scale;

void waitForEnter(const char* prompt) {
  Serial.println(prompt);
  while (!Serial.available()) {
    delay(100);
  }
  while (Serial.available()) Serial.read(); // clear buffer
}

void setup() {
  Serial.begin(9600);
  scale.begin(DOUT, CLK);

  Serial.println("HX711 Calibration Assistant");

  waitForEnter("Press any key to begin...");

  waitForEnter("Remove all weight from the scale. Press any key when ready...");
  scale.tare(); // Zero the scale
  Serial.println("Tare complete.");

  waitForEnter("Place a known weight on the scale, then press any key...");

  Serial.println("Taking average reading. Please wait...");
  float reading = scale.get_units(10);  // Average of 10 readings
  Serial.print("Raw reading: ");
  Serial.println(reading, 2);

  Serial.println("Enter the known mass in kilograms (e.g. 1.00):");

  // Wait for numeric input
  String input = "";
  while (true) {
    while (Serial.available()) {
      char c = Serial.read();
      if (c == '\n' || c == '\r') {
        if (input.length() > 0) break;
      } else {
        input += c;
      }
    }
    if (input.length() > 0) break;
    delay(100);
  }

  float knownMass = input.toFloat();
  Serial.print("You entered: ");
  Serial.print(knownMass, 2);
  Serial.println(" kg");

  float calibration_factor = reading / knownMass;

  Serial.println("\n✅ Calibration complete!");
  Serial.print("Suggested calibration factor: ");
  Serial.println(calibration_factor, 2);
  Serial.println("Copy this value into your main code like so:");
  Serial.print("scale.set_scale(");
  Serial.print(calibration_factor, 2);
  Serial.println(");");
}

void loop() {
  // Nothing – just runs once
}
