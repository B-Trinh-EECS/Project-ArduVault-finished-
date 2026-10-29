#include <IRremote.hpp>
#include <Servo.h>

// -------- PIN SETUP --------
const int IR_PIN = 2;
const int SERVO_PIN = 9;

// -------- SERVO POSITIONS --------
const int LOCK_POS = 0;
const int UNLOCK_POS = 90;

Servo lockServo;

// -------- SEQUENCE SETTINGS --------
const int SEQ_LENGTH = 4;

unsigned long password[SEQ_LENGTH] = {
  0xBD42CA8B,   // Button 1  (replace with your code)
  0xBF40CA8B,   // Button 2  (replace)
  0xBC43CA8B,   // Button 3  (replace)
  0xBE41CA8B    // Button 4  (replace)
};

unsigned long entered[SEQ_LENGTH];
int indexPos = 0;

// ----- ENTER BUTTON CODE -----
unsigned long ENTER_CODE = 0xEB14CA8B;  // Replace with real Enter code

// ----- TIMER SETTINGS -----
unsigned long unlockTime = 0;
const unsigned long AUTO_LOCK_TIME = 5000;  // 5 sec auto-lock

void setup() {
  Serial.begin(9600);

  IrReceiver.begin(IR_PIN, ENABLE_LED_FEEDBACK);

  lockServo.attach(SERVO_PIN);
  lockServo.write(LOCK_POS);
}

void loop() {

  // ---------- AUTO LOCK FUNCTION ----------
  if (unlockTime > 0 && millis() - unlockTime >= AUTO_LOCK_TIME) {
    Serial.println("Auto-locking...");
    lockServo.write(LOCK_POS);
    unlockTime = 0;  // reset timer
  }

  // ---------- READ IR SIGNAL ----------
  if (IrReceiver.decode()) {

    unsigned long code = IrReceiver.decodedIRData.decodedRawData;
    Serial.print("Received: ");
    Serial.println(code, HEX);

    // ---------- IGNORE HOLD/REPEAT SIGNAL ----------
    if (code == 0xFFFFFFFF) {
      Serial.println("Ignored: Repeat/Hold code");
      IrReceiver.resume();
      return;
    }

    // ---------- IGNORE ZERO VALUES ----------
    if (code == 0) {
      Serial.println("Ignored: ZERO VALUE (invalid IR packet)");
      IrReceiver.resume();
      return;
    }

    // ---------- FILTER OUT DUPLICATE BUTTON HOLDS ----------
    if (indexPos > 0 && code == entered[indexPos - 1]) {
      Serial.println("Ignored: Duplicate from held button");
      IrReceiver.resume();
      return;
    }

    // ---------- ENTER BUTTON ----------
    if (code == ENTER_CODE) {

      if (indexPos == SEQ_LENGTH) {

        bool correct = true;
        for (int i = 0; i < SEQ_LENGTH; i++) {
          if (entered[i] != password[i]) {
            correct = false;
            break;
          }
        }

        if (correct) {
          Serial.println("Correct! Unlocking safe...");
          lockServo.write(UNLOCK_POS);
          unlockTime = millis();   // start auto-lock timer
        } else {
          Serial.println("Incorrect sequence.");
        }
      } else {
        Serial.println("Not enough entries!");
      }

      indexPos = 0;  // reset for next attempt
    }

    // ---------- REGULAR BUTTON INPUT ----------
    else {

      if (indexPos < SEQ_LENGTH) {
        entered[indexPos] = code;
        indexPos++;

        Serial.print("Stored in slot ");
        Serial.println(indexPos);
      }
    }

    IrReceiver.resume();
  }
}

