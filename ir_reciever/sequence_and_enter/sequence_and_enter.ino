#include <IRremote.hpp>
#include <Servo.h>

// ---- PIN SETUP ----
const int IR_PIN = 2;
const int SERVO_PIN = 9;

// ---- SERVO POSITIONS ----
const int LOCK_POS = 0;
const int UNLOCK_POS = 90;

Servo lockServo;

// ---- SEQUENCE SETTINGS ----
const int SEQ_LENGTH = 4;

unsigned long password[SEQ_LENGTH] = {
  0xBD42CA8B,   // Button 1  (replace with your code)
  0xBF40CA8B,   // Button 2  (replace)
  0xBC43CA8B,   // Button 3  (replace)
  0xBE41CA8B    // Button 4  (replace)
};

unsigned long entered[SEQ_LENGTH];
int indexPos = 0;

// ---- ENTER BUTTON CODE ----
unsigned long ENTER_CODE = 0xEB14CA8B;   // Replace with your Enter button code

void setup() {
  Serial.begin(9600);

  IrReceiver.begin(IR_PIN, ENABLE_LED_FEEDBACK);

  lockServo.attach(SERVO_PIN);
  lockServo.write(LOCK_POS);
}

void loop() {

  if (IrReceiver.decode()) {

    unsigned long code = IrReceiver.decodedIRData.decodedRawData;
    Serial.print("Code: ");
    Serial.println(code, HEX);

    // -----------------------------
    // 1. ENTER BUTTON PRESSED
    // -----------------------------
    if (code == ENTER_CODE) {

      // If we have exactly 4 inputs
      if (indexPos == SEQ_LENGTH) {

        bool correct = true;

        // Compare entered sequence with password
        for (int i = 0; i < SEQ_LENGTH; i++) {
          if (entered[i] != password[i]) {
            correct = false;
            break;
          }
        }

        if (correct) {
          Serial.println("Correct sequence! Unlocking...");
          lockServo.write(UNLOCK_POS);
        } 
        else {
          Serial.println("Incorrect sequence.");
        }
      }
      else {
        Serial.println("Not enough inputs!");
      }

      // reset input for next time
      indexPos = 0;
    }

    // -----------------------------
    // 2. REGULAR BUTTON INPUT
    // -----------------------------
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
