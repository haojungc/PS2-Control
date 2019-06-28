#include <SoftwareSerial.h>
#include "DCmotor.h"
#include <Wire.h>
MOTOR conveyor;
SoftwareSerial BT(6, 7);

char num ;
void conveyorControl();


void setup() {
  conveyor.setPin(10, 11);
  Serial.begin(9600);
  Serial.println("Enter AT commands:");
  BT.begin(9600);
}


void loop() {
  int inSize;
  char input;

  for (;;) {
    if ((inSize = (BT.available())) > 0) {
      num = (char)BT.read();
    }
    conveyorControl();
  }
delay(30);
}

#define A 0
#define B 1
#define q 2
void conveyorControl() {
  static byte mode = q;
  if (num == 'A') {
    mode = A;
  }
  else if (num == 'B') {
    mode = B;
  }
  else if (num == 'q') {
    mode = q;
  }

  if(mode == A){
    Serial.println("向前");
    conveyor.spin(255, CW);
  }
  else if(mode == B){
    Serial.println("向後");
    conveyor.spin(255, CCW);
  }
  else{
    Serial.println("停");
    conveyor.spin(0);
  }
}