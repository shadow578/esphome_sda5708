#include <Arduino.h>
#include "sdascreen.h"

#define SCREEN_LOAD 12
#define SCREEN_DATA 3
#define SCREEN_SDCLK 13
#define SCREEN_RESET 14

SDAScreen screen(SCREEN_LOAD, SCREEN_DATA, SCREEN_SDCLK, SCREEN_RESET);

const DigitData_t smile = {
    0b00000,
    0b01010,
    0b00000,
    0b00000,
    0b10001,
    0b01110 //
};

void setup()
{
  Serial.begin(115200);

  while (!Serial)
  {
    delay(10);
  }
  delay(5000);

  Serial.println("Initializing screen...");
  screen.Initialize();
}

void loop()
{
  Serial.println("Resetting screen...");
  screen.Reset();

  static bool g_bLowPeakCurrent = false;
  Serial.printf("Setting peak current to %s...\n", g_bLowPeakCurrent ? "12.5%" : "maximum");
  screen.SetPeakCurrent(g_bLowPeakCurrent);
  g_bLowPeakCurrent = !g_bLowPeakCurrent;

  Serial.println("Displaying smiley face on digit 0...");
  screen.WriteDigit(0, smile);
  delay(1000);

  Serial.println("Clear Screen");
  screen.Clear();
  delay(1000);

  Serial.println("Displaying smiley face on digit 0, ramp up brightness...");
  screen.WriteDigit(0, smile);

  for (uint8_t br = 0; br < 8; br++)
  {
    Serial.printf("Setting brightness to %d...\n", br);
    screen.SetBrightness(br);
    delay(1000);
  }

  for (int digit = 0; digit < 8; digit++)
  {
    Serial.printf("Setting digit %d...\n", digit);
    screen.WriteDigit(digit, smile);

    delay(1000);
  }

  delay(5000);
}
