//Car parking system through Blynk 


#define BLYNK_TEMPLATE_ID "TMPL36_-yOmKM"
#define BLYNK_TEMPLATE_NAME "Parking system"
#define BLYNK_AUTH_TOKEN "YOUR_BLYNK_AUTH_TOKEN"

#define BLYNK_PRINT Serial

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Servo.h>

// WiFi credentials
char ssid[] = "YOUR_WIFI_SSID";
char pass[] = "YOUR_WIFI_PASSWORD";

// IR Pins
#define IR1 D0
#define IR2 D6
#define IR3 D7
#define IR_ENTRY D3
#define IR_EXIT D4

// Servo
#define SERVO_PIN D5
Servo myservo;

// LCD
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Blynk Virtual Pins
#define SLOT1_LED V0
#define SLOT2_LED V1
#define SLOT3_LED V2
#define EXIT_COUNT_DISPLAY V5
#define IR_EXIT_LED V3
#define EXIT_GATE_SWITCH V4

// Variables
const int maxSlots = 3;
int currentSlots = maxSlots;
int carsExited = 0;

bool entryDetected = false;
bool lastExitSensorState = HIGH;

BlynkTimer timer;

void setup() {
  Serial.begin(9600);
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  pinMode(IR1, INPUT);
  pinMode(IR2, INPUT);
  pinMode(IR3, INPUT);
  pinMode(IR_ENTRY, INPUT);
  pinMode(IR_EXIT, INPUT);

  myservo.attach(SERVO_PIN);
  myservo.write(0);  // Servo closed

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("  Parking System ");
  lcd.setCursor(0, 1);
  lcd.print("   Initializing  ");
  delay(2000);
  lcd.clear();

  timer.setInterval(500L, checkSlots);
  timer.setInterval(1000L, updateLCD);
  timer.setInterval(1000L, updateBlynk);
  timer.setInterval(300L, monitorExitSensor);
}

// ========== Loop ==========
void loop() {
  Blynk.run();
  timer.run();

  // Entry IR detection
  if (digitalRead(IR_ENTRY) == LOW && !entryDetected) {
    entryDetected = true;
    if (currentSlots > 0) {
      currentSlots--;
      openGate();
    } else {
      lcd.setCursor(0, 0);
      lcd.print(" Parking Full    ");
      lcd.setCursor(0, 1);
      lcd.print(" Can't Enter     ");
      delay(2000);
      lcd.clear();
    }
  }
  if (digitalRead(IR_ENTRY) == HIGH) entryDetected = false;
}

// ========== Exit IR LED and Monitoring ==========
void monitorExitSensor() {
  bool currentExitState = digitalRead(IR_EXIT);
  Blynk.virtualWrite(IR_EXIT_LED, currentExitState == LOW ? 255 : 0);
  lastExitSensorState = currentExitState;
}

// ========== Blynk Button for Exit ==========
BLYNK_WRITE(EXIT_GATE_SWITCH) {
  int gateState = param.asInt();  // 1 = ON, 0 = OFF
  Serial.print("V4 switch state: ");
  Serial.println(gateState);

  if (gateState == 1) {
    if (currentSlots < maxSlots) {
      currentSlots++;
      carsExited++;
      openGate();

      // Optional LCD message
      lcd.setCursor(0, 0);
      lcd.print("Exit via User   ");
      lcd.setCursor(0, 1);
      lcd.print("Gate Opened     ");
      delay(2000);
      lcd.clear();
    } else {
      lcd.setCursor(0, 0);
      lcd.print(" No Cars Parked  ");
      lcd.setCursor(0, 1);
      lcd.print(" Nothing to Exit ");
      delay(2000);
      lcd.clear();
    }

    // Reset the button back to OFF (simulate push)
    Blynk.virtualWrite(EXIT_GATE_SWITCH, 0);
  }
}

#define RESET_BUTTON V6  // Virtual pin for reset button

BLYNK_WRITE(RESET_BUTTON) {
  int resetState = param.asInt();
  if (resetState == 1) {
    currentSlots = maxSlots;
    carsExited = 0;

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("System Reset     ");
    lcd.setCursor(0, 1);
    lcd.print("All Set to Zero  ");
    delay(2000);
    lcd.clear();

    // Update Blynk values
    updateBlynk();

    // Reset switch back to 0 (simulate push)
    Blynk.virtualWrite(RESET_BUTTON, 0);
  }
}

// ========== Gate Control ==========
void openGate() {
  myservo.write(180);
  delay(1000);
  myservo.write(0);
  delay(500);
}

// ========== Update LCD ==========
void updateLCD() {
  lcd.setCursor(0, 0);
  lcd.print("Slots Left: ");
  lcd.print(currentSlots);
  lcd.print("   ");

  lcd.setCursor(0, 1);
  lcd.print("Exited: ");
  lcd.print(carsExited);
  lcd.print("     ");
}

// ========== Update Blynk ==========
void updateBlynk() {
  Blynk.virtualWrite(SLOT1_LED, digitalRead(IR1) == LOW ? 255 : 0);
  Blynk.virtualWrite(SLOT2_LED, digitalRead(IR2) == LOW ? 255 : 0);
  Blynk.virtualWrite(SLOT3_LED, digitalRead(IR3) == LOW ? 255 : 0);
  Blynk.virtualWrite(EXIT_COUNT_DISPLAY, carsExited);
}

// ========== Slot Sensor Status ==========
void checkSlots() {
  updateSlotStatus(digitalRead(IR1), SLOT1_LED, 1);
  updateSlotStatus(digitalRead(IR2), SLOT2_LED, 2);
  updateSlotStatus(digitalRead(IR3), SLOT3_LED, 3);
}

void updateSlotStatus(int sensorValue, int blynkPin, int slotNumber) {
  if (sensorValue == LOW) {
    Serial.print("Slot ");
    Serial.print(slotNumber);
    Serial.println(": Occupied");
  } else {
    Serial.print("Slot ");
    Serial.print(slotNumber);
    Serial.println(": Available");
  }
}