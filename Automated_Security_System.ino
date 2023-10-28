#include <Wire.h>
#include <SPI.h>
#include <MFRC522.h>
#include <LiquidCrystal_I2C.h>

#define RST_PIN 9
#define SS_PIN 10
#define Buzzer 2

const int STEPPER_PIN_1 = 5;
const int STEPPER_PIN_2 = 6;
const int STEPPER_PIN_3 = 8;
const int STEPPER_PIN_4 = 7;

const int STEPS_PER_REVOLUTION = 2048;
const float DEGREES_PER_STEP = 360.0 / STEPS_PER_REVOLUTION;
const int TARGET_DEGREES = 21;  // angle

const int entranceSensorPin = 3;
const int exitSensorPin = 4;
int peopleCount = 0;
int NumberofRotation = 0;

MFRC522 mfrc522(SS_PIN, RST_PIN);

LiquidCrystal_I2C lcd(0x3F, 16, 2);

// Define the target UIDs
byte targetUid1[] = { 0x3B, 0x7D, 0xE6, 0x21 };
byte targetUid2[] = { 0x43, 0xE6, 0x6E, 0xA3 };
byte targetUid3[] = { 0xF3, 0xFA, 0x6F, 0xA3 };

void setup() {
  Serial.begin(9600);
  pinMode(Buzzer, OUTPUT);
  pinMode(exitSensorPin, INPUT);
  pinMode(entranceSensorPin, INPUT);
  pinMode(STEPPER_PIN_1, OUTPUT);
  pinMode(STEPPER_PIN_2, OUTPUT);
  pinMode(STEPPER_PIN_3, OUTPUT);
  pinMode(STEPPER_PIN_4, OUTPUT);

  digitalWrite(Buzzer, LOW);
  SPI.begin();
  mfrc522.PCD_Init();
  delay(4);
  mfrc522.PCD_DumpVersionToSerial();
  Serial.println(F("System is Ready..."));

  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(5, 0);
  lcd.print("SYSTEM");
  lcd.setCursor(1, 1);
  lcd.print("INITIALIZATION");
  delay(2000);
}

void loop() {
  digitalWrite(Buzzer, LOW);
  if (peopleCount > 10) {
    lcd.clear();
    lcd.setCursor(4, 0);
    lcd.print("SCAN ID");
    lcd.setCursor(0, 1);
    lcd.print("Maximum People");
    digitalWrite(Buzzer, HIGH);
  } else {
    lcd.clear();
    lcd.setCursor(4, 0);
    lcd.print("SCAN ID");
    lcd.setCursor(0, 1);
    lcd.print("People In: ");
    lcd.setCursor(11, 1);
    if (peopleCount < 0) {
      peopleCount = 0;
    }
    lcd.print(peopleCount);
  }
  if (digitalRead(entranceSensorPin) == HIGH) {
    //delay(100);  // Debouncing delay to avoid multiple counts for a single person
    if (digitalRead(entranceSensorPin) == HIGH) {
      peopleCount++;
      digitalWrite(Buzzer, LOW);
      //delay(1000);  // Delay to prevent multiple counts for a single person
    }
  }
  // Check the exit sensor
  if (digitalRead(exitSensorPin) == HIGH) {
    //delay(100);  // Debouncing delay to avoid multiple counts for a single person
    peopleCount--;
    digitalWrite(Buzzer, LOW);
    //delay(1000);  // Delay to prevent multiple counts for a single person
  } else {
    digitalWrite(Buzzer, LOW);
  }
  // Print the current number of people inside the house
  Serial.println("People inside the house: " + String(peopleCount));

  if (!mfrc522.PICC_IsNewCardPresent()) {
    return;
  }

  if (!mfrc522.PICC_ReadCardSerial()) {
    return;
  }

  // Check if the scanned card matches any of the target cards
  if (compareUid(mfrc522.uid.uidByte, targetUid1, mfrc522.uid.size)) {
    digitalWrite(Buzzer, HIGH);
    lcd.clear();
    lcd.setCursor(2, 0);
    lcd.print("JAMES OMONDI");
    lcd.setCursor(3, 1);
    lcd.print("ID EXPIRED");
    delay(1000);
  } else if (compareUid(mfrc522.uid.uidByte, targetUid2, mfrc522.uid.size)) {
    digitalWrite(Buzzer, LOW);
    lcd.clear();
    lcd.setCursor(2, 0);
    lcd.print("JOSE MWENDWA");
    lcd.setCursor(0, 1);
    lcd.print("VERIFY FACE ID...");
    delay(1000);
    unsigned long startTime_1 = millis();
    while (millis() - startTime_1 < 10000) {
      if (peopleCount < 10) {
        VerifyFace();
        if (NumberofRotation == 1) {
          lcd.clear();
          lcd.setCursor(2, 0);
          lcd.print("JOSE MWENDWA");
          lcd.setCursor(0, 1);
          lcd.print("STUDENT VERIFIED");
        }
      }
    }
    NumberofRotation = 0;
  } else if (compareUid(mfrc522.uid.uidByte, targetUid3, mfrc522.uid.size)) {
    // digitalWrite(Buzzer, HIGH);
    // lcd.clear();
    // lcd.setCursor(1, 0);
    // lcd.print("MATILDA JOSPHAT");
    // lcd.setCursor(2, 1);
    // lcd.print("PLZ PAY FEES");
    // delay(1000);
    digitalWrite(Buzzer, LOW);
    lcd.clear();
    lcd.setCursor(1, 0);
    lcd.print("MIRIAM KINENE");
    lcd.print("VERIFY FACE ID..");
    delay(1000);
    unsigned long startTime_2 = millis();
    while (millis() - startTime_2 < 10000) {
      if (peopleCount < 10) {
        VerifyFace();
        if (NumberofRotation == 1) {
          lcd.clear();
          lcd.setCursor(2, 0);
          lcd.print("MIRIAM KINENE");
          lcd.setCursor(0, 1);
          lcd.print("LEC VERIFIED");
        }
      }
    }
    NumberofRotation = 0;
  } else {
    digitalWrite(Buzzer, HIGH);
    lcd.clear();
    lcd.setCursor(1, 0);
    lcd.print("CARD NOT FOUND");
  }

  delay(2000);

  // Dump debug info about the card; PICC_HaltA() is automatically called
  mfrc522.PICC_DumpToSerial(&(mfrc522.uid));
}

// Function to compare two arrays
bool compareUid(byte *uid1, byte *uid2, byte size) {
  for (byte i = 0; i < size; i++) {
    if (uid1[i] != uid2[i]) {
      return false;  // Mismatch found
    }
  }
  return true;  // Arrays match
}
void rotateAntiClockwise() {
  int targetSteps = TARGET_DEGREES / DEGREES_PER_STEP;
  const int stepSequence[8][4] = {
    { HIGH, LOW, LOW, HIGH },
    { LOW, LOW, LOW, HIGH },
    { LOW, LOW, HIGH, HIGH },
    { LOW, LOW, HIGH, LOW },
    { LOW, HIGH, HIGH, LOW },
    { LOW, HIGH, LOW, LOW },
    { HIGH, HIGH, LOW, LOW },
    { HIGH, LOW, LOW, LOW }

  };
  for (int i = 0; i < targetSteps; i++) {
    for (int j = 0; j < 8; j++) {
      digitalWrite(STEPPER_PIN_4, stepSequence[j][3]);
      digitalWrite(STEPPER_PIN_3, stepSequence[j][2]);
      digitalWrite(STEPPER_PIN_2, stepSequence[j][1]);
      digitalWrite(STEPPER_PIN_1, stepSequence[j][0]);
      delayMicroseconds(1000);
    }
  }
  delay(500);

  digitalWrite(STEPPER_PIN_4, LOW);
  digitalWrite(STEPPER_PIN_3, LOW);
  digitalWrite(STEPPER_PIN_2, LOW);
  digitalWrite(STEPPER_PIN_1, LOW);
}
void VerifyFace() {
  if (Serial.available() > 0) {
    char receivedChar = Serial.read();

    if (receivedChar == '1') {
      //Serial.println("Received '1': Face Recognized");
      digitalWrite(Buzzer, LOW);  // Turn on LED
      if (NumberofRotation == 0 && peopleCount < 10) {
        rotateAntiClockwise();
        NumberofRotation = 1;
      }
    } else if (receivedChar == '2') {
      //Serial.println("Received '2': Face Not Recognized");
      digitalWrite(Buzzer, HIGH);
    } else if (receivedChar == '3') {
      // Code to execute when '3' is received
      //Serial.println("Received '3': No Face Detected");
      digitalWrite(Buzzer, LOW);  // Turn off LED
    }
  }
}