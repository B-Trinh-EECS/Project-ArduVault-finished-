#include <IRremote.hpp>
#include <Servo.h>
#include <EEPROM.h>

// -------- PIN SETUP --------
const int IR_PIN = 2;
const int SERVO_PIN = 9;

// -------- SERVO POSITIONS --------
const int LOCK_POS = 0;
const int UNLOCK_POS = 90;

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
unsigned long ENTER_CODE = 0xEC13CA8B; //enter button in the middle

// ----- TIMER SETTINGS -----
unsigned long unlockTime = 0;
const unsigned long AUTO_LOCK_TIME = 1000; //5s auto-lock

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

// EEPROM password buffer
unsigned long storedPass[SEQ_LENGTH];

// --------------------------------------------------
//  SETUP
// --------------------------------------------------

void setup() {
  Serial.begin(9600);
  IrReceiver.begin(IR_PIN, ENABLE_LED_FEEDBACK);

  lockServo.attach(SERVO_PIN);
  lockServo.write(LOCK_POS);

  // Initialize EEPROM if empty
  if (isEEPROMBlank()) {
    Serial.println("EEPROM empty → Writing default password...");
    writePasswordToEEPROM(defaultPass);
  }

  // Load password into RAM
  readPasswordFromEEPROM(storedPass);

  Serial.println("Loaded Password from EEPROM:");
  for (int i = 0; i < SEQ_LENGTH; i++) {
    Serial.println(storedPass[i], HEX);
  }
}

// --------------------------------------------------
//  MAIN LOOP
// --------------------------------------------------

void loop() {

  // ---------- AUTO LOCK ----------
  if (unlockTime > 0 && millis() - unlockTime >= AUTO_LOCK_TIME) {
    Serial.println("Auto-locking...");
    lockServo.write(LOCK_POS);
    unlockTime = 0;
  }

  // ---------- IR READ ----------
  if (IrReceiver.decode()) {

    unsigned long code = IrReceiver.decodedIRData.decodedRawData;
    Serial.print("IR Received: ");
    Serial.println(code, HEX);

    // ---------- INVALID REPEAT SIGNAL ----------
    if (code == 0xFFFFFFFF) {
      Serial.println("Ignored: Repeat/Hold code");
      IrReceiver.resume();
      return;
    }

    // ---------- FILTER ZERO VALUE ----------
    if (code == 0) {
      Serial.println("Ignored: Zero value");
      IrReceiver.resume();
      return;
    }

    // ---------- FILTER HELD BUTTON DUPLICATE ----------
    if (indexPos > 0 && code == entered[indexPos - 1]) {
      Serial.println("Ignored: Button held duplicate");
      IrReceiver.resume();
      return;
    }

    // ---------- ENTER BUTTON ----------
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
        } else {
          Serial.println("Incorrect Sequence.");
        }
      } else {
        Serial.println("Sequence incomplete!");
      }

      indexPos = 0;  // reset either way
    }

    // ---------- REGULAR BUTTON INPUT ----------
    else {

      if (indexPos < SEQ_LENGTH) {
        entered[indexPos] = code;
        indexPos++;

        Serial.print("Stored at slot ");
        Serial.println(indexPos);
      }
    }

    IrReceiver.resume();
  }
}
