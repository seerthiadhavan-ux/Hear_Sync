#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// ========================================================
// PIN DEFINITIONS
// ========================================================
#define LEFT_PIN      18  // Blue LED  - Left side motor
#define CENTER_PIN    19  // Yellow LED - Center / North alert
#define RIGHT_PIN     23  // Red LED   - Right side motor

#define DIR_PIN       34  // Compass Direction Knob directly under Center LED
#define SIREN_BUTTON  14  // Pushbutton to trigger sound siren alert

#define LCD_ADDRESS   0x27
#define LCD_COLS      16
#define LCD_ROWS      2

LiquidCrystal_I2C lcd(LCD_ADDRESS, LCD_COLS, LCD_ROWS);

// ========================================================
// COMPASS DIRECTION CALIBRATION
// ========================================================
// 0 - 1700:      LEFT (West / 9 o'clock)  -> Blue LED
// 1700 - 2400:   NORTH / SOUTH (Center)    -> Yellow LED
// 2400 - 4095:   RIGHT (East / 3 o'clock)  -> Red LED
const int DIR_NORTH_MIN = 1700;
const int DIR_NORTH_MAX = 2400;

unsigned long lastSampleTime = 0;
unsigned long lastSerialTime = 0;

int currentDirRaw = 0;
String cachedLine1 = "";
String cachedLine2 = "";

// ========================================================
// FLICKER-FREE LCD HELPER
// ========================================================
void updateLCD(String line1, String line2, bool forceClear = false) {
  while (line1.length() < 16) line1 += ' ';
  if (line1.length() > 16) line1 = line1.substring(0, 16);

  while (line2.length() < 16) line2 += ' ';
  if (line2.length() > 16) line2 = line2.substring(0, 16);

  if (!forceClear && line1 == cachedLine1 && line2 == cachedLine2) {
    return;
  }

  if (forceClear) {
    lcd.clear();
  }

  lcd.setCursor(0, 0);
  lcd.print(line1);
  lcd.setCursor(0, 1);
  lcd.print(line2);

  cachedLine1 = line1;
  cachedLine2 = line2;
}

// ========================================================
// HAPTIC LED DRIVER
// ========================================================
void setSoleLED(int activePin) {
  digitalWrite(LEFT_PIN,   activePin == LEFT_PIN   ? HIGH : LOW);
  digitalWrite(CENTER_PIN, activePin == CENTER_PIN ? HIGH : LOW);
  digitalWrite(RIGHT_PIN,  activePin == RIGHT_PIN  ? HIGH : LOW);
}

void allOff() {
  digitalWrite(LEFT_PIN, LOW);
  digitalWrite(CENTER_PIN, LOW);
  digitalWrite(RIGHT_PIN, LOW);
}

// Rapid 3-pulse urgent siren pattern on the active direction LED
void pulseSirenAlert(int activePin) {
  allOff();
  for (int i = 0; i < 3; i++) {
    digitalWrite(activePin, HIGH);
    delay(80);
    digitalWrite(activePin, LOW);
    delay(80);
  }
}

// ========================================================
// SETUP
// ========================================================
void setup() {
  Serial.begin(115200);
  delay(100);

  pinMode(LEFT_PIN, OUTPUT);
  pinMode(CENTER_PIN, OUTPUT);
  pinMode(RIGHT_PIN, OUTPUT);
  pinMode(DIR_PIN, INPUT);
  pinMode(SIREN_BUTTON, INPUT_PULLUP);

  allOff();

  Wire.begin(21, 22);
  lcd.init();
  lcd.backlight();
  lcd.clear();

  updateLCD("HEARSYNC COMPASS", "READY...", true);
  delay(1000);

  Serial.println("========================================");
  Serial.println("     HEARSYNC COMPASS SOUND SYSTEM      ");
  Serial.println("========================================");
  Serial.println("LEFT  (West) : ADC < 1700  -> Blue LED  ");
  Serial.println("NORTH/SOUTH  : 1700 - 2400 -> Yellow LED");
  Serial.println("RIGHT (East) : ADC > 2400  -> Red LED   ");
  Serial.println("========================================");
}

// ========================================================
// MAIN LOOP
// ========================================================
void loop() {
  unsigned long now = millis();

  if (now - lastSampleTime >= 50) {
    lastSampleTime = now;

    currentDirRaw = analogRead(DIR_PIN);
    bool sirenTriggered = (digitalRead(SIREN_BUTTON) == LOW);

    int activePin;
    String dirLabel;
    String arrow;

    // Strict Compass Zone Detection:
    if (currentDirRaw >= DIR_NORTH_MIN && currentDirRaw <= DIR_NORTH_MAX) {
      // NORTH / SOUTH (Pointing straight up or center) -> ONLY Middle Yellow LED glows!
      activePin = CENTER_PIN;
      dirLabel  = "NORTH / CENTER";
      arrow     = "[ ^ ]";
    } else if (currentDirRaw < DIR_NORTH_MIN) {
      // LEFT (West / pointing left) -> ONLY Blue Left LED glows!
      activePin = LEFT_PIN;
      dirLabel  = "WEST / LEFT   ";
      arrow     = "[<--]";
    } else {
      // RIGHT (East / pointing right) -> ONLY Red Right LED glows!
      activePin = RIGHT_PIN;
      dirLabel  = "EAST / RIGHT  ";
      arrow     = "[-->]";
    }

    if (sirenTriggered) {
      // Emergency Siren Alert Pulsing on the active direction
      updateLCD("!! SIREN ALERT !!", arrow + " " + dirLabel);
      pulseSirenAlert(activePin);
    } else {
      // Normal Compass Direction Tracking:
      // Exactly ONE LED glows matching the dial pointer
      setSoleLED(activePin);
      updateLCD("DIR: " + dirLabel, arrow + " SOUND READY");
    }
  }

  // Serial Monitor (every 500 ms)
  if (now - lastSerialTime >= 500) {
    lastSerialTime = now;

    String dirStr = (currentDirRaw >= DIR_NORTH_MIN && currentDirRaw <= DIR_NORTH_MAX) ? "NORTH (CENTER)" :
                    (currentDirRaw < DIR_NORTH_MIN) ? "LEFT (WEST)" : "RIGHT (EAST)";

    Serial.print("Knob ADC: ");
    Serial.print(currentDirRaw);
    Serial.print(" | Compass Direction: ");
    Serial.print(dirStr);
    Serial.println(" -> Active LED is ON");
  }
}
