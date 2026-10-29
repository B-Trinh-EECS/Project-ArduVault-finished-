#include <IRremote.hpp>
#include <Servo.h>
#include <EEPROM.h>

//due to interferrence, the code will get ir signal that is out of range
//light works, but due to thin pin will make the light seem to not work

// -------- PIN SETUP --------
const int IR_PIN = 2;
const int SERVO_PIN = 9;
const int GREEN_LED = 5;
const int RED_LED = 6;

// -------- SERVO POSITIONS --------
const int LOCK_POS = 90;
const int UNLOCK_POS = 0;

Servo lockServo;

// -------- PASSWORD SETTINGS --------
const int SEQ_LENGTH = 4;

unsigned long entered[SEQ_LENGTH];
int indexPos = 0;

// ----- YOUR NEW BUTTON CODES -----
unsigned long BTN_LEFT  = 0xBD42CA8B;
unsigned long BTN_UP    = 0xBF40CA8B;
unsigned long BTN_RIGHT = 0xBC43CA8B;
unsigned long BTN_DOWN  = 0xBE41CA8B;

// default EEPROM password (these 4)
unsigned long defaultPass[SEQ_LENGTH] = {
  BTN_LEFT,
  BTN_UP,
  BTN_RIGHT,
  BTN_DOWN
};

// ENTER Button
unsigned long ENTER_CODE = 0xEC13CA8B;

// ----- TIMER SETTINGS -----
unsigned long unlockTime = 0;
const unsigned long AUTO_LOCK_TIME = 500;  // 20 seconds

// EEPROM password storage buffer
unsigned long storedPass[SEQ_LENGTH];

// --------------------------------------------------
//  EEPROM HELPERS
// --------------------------------------------------

void writePasswordToEEPROM(unsigned long *pass) {
  for (int i = 0; i < SEQ_LENGTH; i++) {
    EEPROM.put(i * sizeof(unsigned long), pass[i]);
  }
}

void readPasswordFromEEPROM(unsigned long *pass) {
  for (int i = 0; i < SEQ_LENGTH; i++) {
    EEPROM.get(i * sizeof(unsigned long), pass[i]);
  }
}

bool isEEPROMBlank() {
  for (int i = 0; i < SEQ_LENGTH; i++) {
    unsigned long test;
    EEPROM.get(i * sizeof(unsigned long), test);
    if (test != 0xFFFFFFFF && test != 0) return false;
  }
  return true;
}

// --------------------------------------------------
//  LED HELPERS
// --------------------------------------------------

void blinkGreenFast() {
  digitalWrite(GREEN_LED, HIGH);
  delay(50);
  digitalWrite(GREEN_LED, LOW);
}

void blinkGreenUnlock() {
  for (int i = 0; i < 10; i++) {
    digitalWrite(GREEN_LED, HIGH);
    delay(1000);
    digitalWrite(GREEN_LED, LOW);
    delay(1000);
  }
}

void blinkRedError() {
  for (int i = 0; i < 3; i++) {
    digitalWrite(RED_LED, HIGH);
    delay(200);
    digitalWrite(RED_LED, LOW);
    delay(200);
  }
}

// --------------------------------------------------
//  SETUP
// --------------------------------------------------

void setup() {
  Serial.begin(9600);
  IrReceiver.begin(IR_PIN, ENABLE_LED_FEEDBACK);

  pinMode(GREEN_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);

  lockServo.attach(SERVO_PIN);
  lockServo.write(LOCK_POS);

  if (isEEPROMBlank()) {
    writePasswordToEEPROM(defaultPass);
  }

  readPasswordFromEEPROM(storedPass);

  Serial.println("READY");
}

// --------------------------------------------------
//  MAIN LOOP
// --------------------------------------------------

void loop() {

  // ---------- AUTO LOCK ----------
  if (unlockTime > 0 && millis() - unlockTime >= AUTO_LOCK_TIME) {
    lockServo.write(LOCK_POS);
    unlockTime = 0;
  }

  // ---------- IR READ ----------
  if (IrReceiver.decode()) {

    unsigned long code = IrReceiver.decodedIRData.decodedRawData;
    Serial.print("IR Received: ");
    Serial.println(code, HEX);

    // ----- THROW OUT BAD SIGNALS -----
    if (code == 0 || code == 0xFFFFFFFF) {
      IrReceiver.resume();
      return;
    }

    // ----- VALID CODE → BLINK GREEN -----
    blinkGreenFast();

    // ----- BLOCK HELD BUTTONS -----
    if (indexPos > 0 && code == entered[indexPos - 1]) {
      Serial.println("Ignored: Duplicate held button.");
      IrReceiver.resume();
      return;
    }

    // ----- ENTER BUTTON -----
    if (code == ENTER_CODE) {

      if (indexPos == SEQ_LENGTH) {
        bool correct = true;
        for (int i = 0; i < SEQ_LENGTH; i++) {
          if (entered[i] != storedPass[i]) {
            correct = false;
            break;
          }
        }

        if (correct) {
          Serial.println("Password Correct! Unlocking...");
          lockServo.write(UNLOCK_POS);
          unlockTime = millis();

          blinkGreenUnlock();   // <-- green flashing WHILE unlocking
        } else {
          Serial.println("Incorrect Password!");
          blinkRedError();
        }
      } else {
        Serial.println("Sequence incomplete.");
        blinkRedError();
      }

      indexPos = 0;
      IrReceiver.resume();
      return;
    }

    // ----- STORE REGULAR BUTTON -----
    if (indexPos < SEQ_LENGTH) {
      entered[indexPos] = code;
      indexPos++;
    }

    IrReceiver.resume();
  }
}


