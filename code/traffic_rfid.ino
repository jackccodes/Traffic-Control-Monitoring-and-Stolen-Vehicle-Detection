//Optimized code for Traffic control monitoring and stolen vehicle detection

#include <SPI.h>
#include <MFRC522.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// RFID Module
#define SS_PIN 10
#define RST_PIN 9
MFRC522 rfid(SS_PIN, RST_PIN);
int count1 = 0, count2 = 0, count3 = 0;


// I2C LCD
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Traffic Light Pins
#define RED1 2
#define GREEN1 3
#define RED2 4
#define GREEN2 5
#define RED3 6
#define GREEN3 7
#define BUZZER 8

// IR Sensors
#define IR1 A0
#define IR2 A1
#define IR3 A2

// RFID Stolen Vehicle ID
String stolenVehicleUID = "360FFD";

// Rush hour threshold
int rushThreshold = 3;
int normalGreenTime = 5000;
int rushGreenTime = 3000;
int transitionRedTime = 1000;

// Lane status
String laneStatus = "L1 CL, L2 CL, L3 CL";

void setup() {
  Serial.begin(9600);
  SPI.begin();
  rfid.PCD_Init();

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Traffic System");
  delay(1000);
  lcd.clear();
  pinMode(RED1, OUTPUT); pinMode(GREEN1, OUTPUT);
  pinMode(RED2, OUTPUT); pinMode(GREEN2, OUTPUT);
  pinMode(RED3, OUTPUT); pinMode(GREEN3, OUTPUT);
  pinMode(BUZZER, OUTPUT);

  pinMode(IR1, INPUT);
  pinMode(IR2, INPUT);
  pinMode(IR3, INPUT);

  resetSignals();
}

void loop() {
  // Prioritize RFID Reading
  checkRFID();
  
  checkTrafficMode();

  // Display Traffic Count
  lcd.setCursor(0, 0);
  lcd.print("Traffic Count:");
  lcd.setCursor(0, 1);
  lcd.print(count1); lcd.print(",");
  lcd.print(count2); lcd.print(",");
  lcd.print(count3);
  delay(500);
  lcd.clear();

}

// Reset all Traffic Lights
void resetSignals() {
  checkRFID();
 
  digitalWrite(RED1, HIGH); digitalWrite(GREEN1, LOW);
  digitalWrite(RED2, HIGH); digitalWrite(GREEN2, LOW);
  digitalWrite(RED3, HIGH); digitalWrite(GREEN3, LOW);
}

// Normal Traffic Controller
void normalTrafficControl() {
  checkRFID();
  updateLaneStatus("L1 OP, L2 CL, L3 CL");
  controlTraffic(GREEN1, RED1, normalGreenTime);

  checkRFID();
  updateLaneStatus("L1 CL, L2 OP, L3 CL");
  controlTraffic(GREEN2, RED2, normalGreenTime);

  checkRFID();
  updateLaneStatus("L1 CL, L2 CL, L3 OP");
  controlTraffic(GREEN3, RED3, normalGreenTime);
}

// Rush Hour - Handle Congestion Dynamically
void handleRushHour(int c1, int c2, int c3) 
{
  checkRFID();
  while (c1 >= rushThreshold || c2 >= rushThreshold || c3 >= rushThreshold) {
    if (c1 >= rushThreshold && c2 >= rushThreshold && c3 >= rushThreshold) {
      // All three lanes are congested, cycle through them properly
      switchLane(1, true);
      switchLane(2, true);
      switchLane(3, true);
    } 
    else if (c1 >= rushThreshold && c2 >= rushThreshold) {
      // Two lanes congested, switch every 5s, the inactive lane is red
      switchLane(1, true);
      switchLane(2, true);
    } 
    else if (c2 >= rushThreshold && c3 >= rushThreshold) {
      switchLane(2, true);
      switchLane(3, true);
    } 
    else if (c1 >= rushThreshold && c3 >= rushThreshold) {
      switchLane(1, true);
      switchLane(3, true);
    } 
    else {
      // Only one lane is congested, keep it green until cleared and turn off its red light
      if (c1 >= rushThreshold) keepLaneGreen(1);
      if (c2 >= rushThreshold) keepLaneGreen(2);
      if (c3 >= rushThreshold) keepLaneGreen(3);
    }

    // Update traffic counts before looping again
    c1 = readTraffic(IR1);
    c2 = readTraffic(IR2);
    c3 = readTraffic(IR3);
    resetSignals();
    checkRFID();
  }
}

// Switch Lanes Dynamically
void switchLane(int lane, bool rotating) {
  checkRFID();
  resetSignals(); // Turn all red before switching

  if (lane == 1) {
    updateLaneStatus("L1 OP, L2 RED, L3 RED");
    digitalWrite(GREEN1, HIGH);
    digitalWrite(RED1, LOW);
    digitalWrite(RED2, HIGH);
    digitalWrite(RED3, HIGH);
    checkRFID();
  } 
  else if (lane == 2) {
    updateLaneStatus("L1 RED, L2 OP, L3 RED");
    digitalWrite(GREEN2, HIGH);
    digitalWrite(RED2, LOW);
    digitalWrite(RED1, HIGH);
    digitalWrite(RED3, HIGH);
    checkRFID();
  } 
  else if (lane == 3) {
    updateLaneStatus("L1 RED, L2 RED, L3 OP");
    digitalWrite(GREEN3, HIGH);
    digitalWrite(RED3, LOW);
    digitalWrite(RED1, HIGH);
    digitalWrite(RED2, HIGH);
    checkRFID();
  }

  if (rotating) {
    delay(5000); // Hold green for 5s before switching
    resetSignals();
  }
}

// Keep a Lane Green Until Cleared
void keepLaneGreen(int lane) {
  checkRFID();
  resetSignals(); // Turn all red before activating this lane

  if (lane == 1) {
    updateLaneStatus("L1 OP, L2 CL, L3 CL");
    digitalWrite(GREEN1, HIGH);
    digitalWrite(RED1, LOW);
    digitalWrite(RED2, HIGH);
    digitalWrite(RED3, HIGH);
    while (readTraffic(IR1) >= rushThreshold) delay(1000);
    checkRFID();
  } 
  else if (lane == 2) {
    updateLaneStatus("L1 CL, L2 OP, L3 CL");
    digitalWrite(GREEN2, HIGH);
    digitalWrite(RED2, LOW);
    digitalWrite(RED1, HIGH);
    digitalWrite(RED3, HIGH);
    while (readTraffic(IR2) >= rushThreshold) delay(1000);
    checkRFID();
  } 
  else if (lane == 3) {
    updateLaneStatus("L1 CL, L2 CL, L3 OP");
    digitalWrite(GREEN3, HIGH);
    digitalWrite(RED3, LOW);
    digitalWrite(RED1, HIGH);
    digitalWrite(RED2, HIGH);
    while (readTraffic(IR3) >= rushThreshold) delay(1000);
    checkRFID();
  }

  resetSignals(); // Turn all red when cleared
}

// Update Lane Status
void updateLaneStatus(String status) {
  checkRFID();
  laneStatus = status;
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(status);
  delay(1000);
}

// Read IR Sensor for Traffic Density
int readTraffic(int sensor) {
  checkRFID();
  int count = 0;
  for (int i = 0; i < 5; i++) {
    if (digitalRead(sensor) == LOW) count++;
    delay(100);
  }
  return count;
}
// *Continuous RFID Check*
void checkRFID() {
  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) return;

  String scannedUID = "";
  for (byte i = 0; i < rfid.uid.size; i++) {
    scannedUID += String(rfid.uid.uidByte[i], HEX);
  }
  scannedUID.toUpperCase();

  // *Override the LCD display immediately*
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("RFID Detected:");
  lcd.setCursor(0, 1);
  lcd.print(scannedUID);
  
  Serial.print("Scanned UID: ");
  Serial.println(scannedUID);

  // *Stop all traffic lights momentarily*
  resetSignals();

  if (scannedUID == stolenVehicleUID) {
    lcd.setCursor(0, 1);
    lcd.print("STOLEN VEHICLE!");
    digitalWrite(BUZZER, HIGH);
    delay(3000);
    digitalWrite(BUZZER, LOW);
  } else {
    lcd.setCursor(0, 1);
    lcd.print("Access Granted");
    delay(2000);
  }

  delay(2000);
  lcd.clear();
  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();

  // *Restore previous lane status and continue traffic control*
  updateLaneStatus(laneStatus);
}
void controlTraffic(int greenPin, int redPin, int duration) {
  checkRFID();
  digitalWrite(redPin, LOW);
  digitalWrite(greenPin, HIGH);
  delay(duration);
  digitalWrite(greenPin, LOW);
  digitalWrite(redPin, HIGH);
  delay(transitionRedTime);
}

void checkTrafficMode() {
  checkRFID();
  int count1 = readTraffic(IR1);
  int count2 = readTraffic(IR2);
  int count3 = readTraffic(IR3);

  lcd.setCursor(0, 0);
  lcd.print("Traffic Count:");
  lcd.setCursor(0, 1);
  lcd.print(count1); lcd.print(",");
  lcd.print(count2); lcd.print(",");
  lcd.print(count3);
  delay(500);
  lcd.clear();

  if (count1 >= rushThreshold || count2 >= rushThreshold || count3 >= rushThreshold) {
    lcd.setCursor(0, 0);
    lcd.print("RUSH MODE ACTIVE");
    delay(500);
    lcd.clear();
    handleRushHour(count1, count2, count3);
    
  } else {
    lcd.setCursor(0, 0);
    lcd.print("NORMAL MODE");
    delay(500);
    lcd.clear();
    normalTrafficControl();
  }
}
