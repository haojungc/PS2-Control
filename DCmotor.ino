#include "DCmotor.h"

  MOTOR::MOTOR(){
    speed = 0; 
    direction = CW;
  }
  
  MOTOR::MOTOR(int pin1, int pin2): speed(0), direction(CW){
    this->pin1 = pin1;
    this->pin2 = pin2;
    pinMode(pin1, OUTPUT);
    pinMode(pin2, OUTPUT);
  }

  void MOTOR::setPin(int pin1, int pin2){
    this->pin1 = pin1;
    this->pin2 = pin2;
    pinMode(pin1, OUTPUT);
    pinMode(pin2, OUTPUT);
  }

  void MOTOR::setSpeed(int speed){
    this->speed = speed;
  }
  void MOTOR::setDirection(bool direction){
    this->direction = direction;
  }
  
  void MOTOR::spin(){
    if(direction == CCW){
      analogWrite(pin1, speed);
      analogWrite(pin2, 0);
    }
    else if(direction == CW){
      analogWrite(pin1, 0);
      analogWrite(pin2, speed);
    }
  }

  void MOTOR::spin(int speed){
    setSpeed(speed);
    
    if(direction == CCW){
      analogWrite(pin1, speed);
      analogWrite(pin2, 0);
    }
    else if(direction == CW){
      analogWrite(pin1, 0);
      analogWrite(pin2, speed);
    }
  }
  
  void MOTOR::spin(int speed, bool direction){
    setSpeed(speed);
    setDirection(direction);
    
    if(direction == CCW){
      analogWrite(pin1, speed);
      analogWrite(pin2, 0);
    }
    else if(direction == CW){
      analogWrite(pin1, 0);
      analogWrite(pin2, speed);
    }
  }
