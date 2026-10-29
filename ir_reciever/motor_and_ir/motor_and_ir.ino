#include <IRremote.hpp>
#include <Servo.h>

// Pins
const int IR_PIN = 2;
const int SERVO_PIN = 9;

// Servo positions
const int LOCK_POS = 0;
const int UNLOCK_POS = 90;

Servo servo;

// Replace these with your remote’s actual codes
unsigned long BUTTON_UNLOCK = 0xBD42CA8B;  // example
unsigned long BUTTON_LOCK   = 0xBC43CA8B;  // example

void setup() {
  Serial.begin(9600);

  IrReceiver.begin(IR_PIN, ENABLE_LED_FEEDBACK);

  servo.attach(SERVO_PIN);
  servo.write(LOCK_POS);   // start locked
}

void loop() {

  if (IrReceiver.decode()) {

    unsigned long code = IrReceiver.decodedIRData.decodedRawData;
    Serial.println(code, HEX);  // Print code so you can see your real buttons

    // Unlock action
    if (code == BUTTON_UNLOCK) {
      servo.write(UNLOCK_POS);
    }

    // Lock action
    if (code == BUTTON_LOCK) {
      servo.write(LOCK_POS);
    }

    IrReceiver.resume();
  }
}

