#include <IRremote.hpp>

const int IR_RECEIVE_PIN = 3;

void setup() {
  Serial.begin(9600);
  Serial.println("IR Receiver test starting...");
  IrReceiver.begin(IR_RECEIVE_PIN, ENABLE_LED_FEEDBACK);
}

void loop() {
  if (IrReceiver.decode()) {
    Serial.print("Data: ");
    Serial.println(IrReceiver.decodedIRData.decodedRawData, HEX);
    IrReceiver.resume();
  }
  /*button:
  left: BD42CA8B
  right: BC43CA8B
  up: BF40CA8B
  down: BE41CA8B
  enter: EB14CA8B
  */
}

