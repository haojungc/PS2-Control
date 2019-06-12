/* Latest Update: 2019/6/13 00:00 */
/* New Features: improve inverse kinematic*/

#include <Servo.h>
#include "PS2X_lib.h"
#include "DCmotor.h"
#include "Arm_Movement.h"

PS2X ps2x;
Servo gripper, updown, backforward;
MOTOR motorA, motorB, motorC;
//MOTOR conveyor;

#define FAST true
#define SLOW false
//#define GRIPPER 44
#define BACKFORWARD 45
#define UPDOWN 46
//#define ELECTROMAGNET 47
#define NAN -1

// gripper pins
const int IN1 = 42;
const int IN2 = 43;
const int ENA = 44;

// gripper & arm positions
const int gInit = 90;
const int udMiddle = 95;	// 80
const int bfMiddle = 105; // don't be larger than 125
const int udHigh = 35;
const int bfHigh = 65;
const int udLow = 60;
const int bfLow = 20;

// arm initial position
int gpos = gInit, udpos = udMiddle, bfpos = bfMiddle;

byte x, y, prev_x = 128, prev_y = 127, prev_prev_x = 128, prev_prev_y = 127;
int error = 0; 
byte type = 0;
byte vibrate = 0;
int val = 0;
int increment = 2;
bool mode = FAST;
int vibrate_count = 2;
bool first_vibrate = true;

int now, last_read = 0;
int rotateTime, prev_rotateTime = 0;

// DC Motor Speed
int fullspeed = 192;
int halfspeed = 96;

// inverse kinematic
double arm_x;

/* Function Prototype */
//void setAngle(int, int);
void toMiddle();
void toHigh();
void toLow();
void carMovement(); // read LX & LY
void rotate();
void gripControl();
void armControl();
//void conveyorControl();
//void electromagnetControl();
void switchMode();

void setup(){
 Serial.begin(57600);

 error = ps2x.config_gamepad(13,11,10,12, true, true);   //setup pins and settings:  GamePad(clock, command, attention, data, Pressures?, Rumble?) check for error
 
 if(error == 0){
  Serial.println("Found Controller, configured successful");
  Serial.println("Try out all the buttons, X will vibrate the controller, faster as you press harder;");
  Serial.println("holding L1 or R1 will print out the analog stick values.");
  Serial.println("Go to www.billporter.info for updates and to report bugs.");
 }
   
  else if(error == 1)
   Serial.println("No controller found, check wiring, see readme.txt to enable debug. visit www.billporter.info for troubleshooting tips");
   
  else if(error == 2)
   Serial.println("Controller found but not accepting commands. see readme.txt to enable debug. Visit www.billporter.info for troubleshooting tips");
   
  else if(error == 3)
   Serial.println("Controller refusing to enter Pressures mode, may not support it. ");
   
   type = ps2x.readType(); 
     switch(type) {
       case 0:
        Serial.println("Unknown Controller type");
       break;
       case 1:
        Serial.println("DualShock Controller Found");
       break;
       case 2:
         Serial.println("GuitarHero Controller Found");
       break;
     }

	/* Set Pin */
	// set gripper pin
	pinMode(IN1, OUTPUT);
	pinMode(IN2, OUTPUT);
	pinMode(ENA, OUTPUT);
  
	// set gripper & arm pins
	//gripper.attach(GRIPPER);
	backforward.attach(BACKFORWARD);
	updown.attach(UPDOWN);

	// set DC motor pin
	motorA.setPin(2, 3);
	motorB.setPin(4, 5);
	motorC.setPin(6, 7);
	//conveyor.setPin(8, 9);

	/* Initialize */
	// DC motor
	motorA.spin(0);
	motorB.spin(0);
	motorC.spin(0);

	// Arm
	//gripper.write(gpos);
	backforward.write(bfMiddle);
	updown.write(udMiddle);

	// Electromagnet
	//pinMode(ELECTROMAGNET, OUTPUT);
}

void loop(){
	if(error == 1) // skip loop if no controller found
		return; 
  
	if(type == 2){} // Guitar Hero Controller

	else { //DualShock Controller
		ps2x.read_gamepad(false, vibrate); // read controller and set large motor to spin at 'vibrate' speed

		/* switch mode: fast & slow */
		switchMode();	// set vibrate values

		/* gripper: Square & Triangle */
		gripControl();

		/* Arm: UP, DOWN, LEFT, RIGHT, Circle, L2, R2 */
		armControl();
    
		/* Conveyor Spin: L1, R1 */
		//conveyorControl();
    
		/* Electromagnet: L2, R2 */
		//electromagnetControl();

		/* DC Motor Spin */
		carMovement();

		/* Wheel spin CW/CCW */
		rotateTime = millis();
		if(rotateTime - prev_rotateTime > 15){
			rotate();
			prev_rotateTime = rotateTime;
		}
	}
	delay(15); 
}

void switchMode(){
	// set vibrate values
	if(vibrate_count == 1){
		now = millis();
		if(now - last_read > 500){
			vibrate = 0;
			vibrate_count = 0;
		}
	}
	else if(vibrate_count == 2){
		now = millis();
		if(first_vibrate){
			if(now - last_read > 400){
				first_vibrate = false;
				vibrate = 0;
				last_read = now;
			}
		}
		else{
			if(now - last_read > 200){
				first_vibrate = true;
				vibrate = 255;
				vibrate_count--;
				last_read = now;
			}
		}
	}

	// switch mode: fast & slow
	if(ps2x.ButtonReleased(PSB_L3) || ps2x.ButtonReleased(PSB_R3)){
		if(mode == FAST){
			Serial.println("Slow Mode");
			mode = SLOW;
			vibrate_count = 1;
			increment = 1;
			fullspeed = 80;
			halfspeed = 40;
			vibrate = 255;
			last_read = millis();
		}
		else{
			Serial.println("Fast Mode");
			mode = FAST;
			vibrate_count = 2;
			increment = 2;
			fullspeed = 192;
			halfspeed = 96;
			vibrate = 255;
			last_read = millis();
		}	
	}
	/*if(ps2x.ButtonPressed(PSB_SELECT)){
		if(mode == FAST){
			//Serial.println("Slow Mode");
			mode = SLOW;
			increment = 1;
			fullspeed = 80;
			halfspeed = 40;
		}
		else{
			//Serial.println("Fast Mode");
			mode = FAST;
			increment = 2;
			fullspeed = 192;
			halfspeed = 96;
		}	
	}*/
}

void gripControl(){	
	// gripper grip: press green triangle
	if(ps2x.Button(PSB_GREEN)){
		//Serial.println("Green pressed: Gripping");
		digitalWrite(IN1, HIGH);
		digitalWrite(IN2, LOW);
		analogWrite(ENA, 255);
		//gripper.write(0);
	}
	// gripper release: press pink square
	else if(ps2x.Button(PSB_PINK)){
		//Serial.println("Pink pressed: Releasing");
		digitalWrite(IN1, LOW);
		digitalWrite(IN2, HIGH);
		analogWrite(ENA, 255);
		//gripper.write(180);
	}
	else{
		digitalWrite(IN1, LOW);
		digitalWrite(IN2, LOW);
		analogWrite(ENA, 0);
		//gripper.write(90);
	}
}

void armControl(){
	static int prev_button = NAN;
	
	// go to original position
	if(ps2x.ButtonPressed(PSB_RED)){
		//Serial.println("CIRCLE PRESSED");
		motorA.spin(0);
		motorB.spin(0);
		motorC.spin(0);
		toMiddle();
		prev_button = PSB_RED;
	}

	// go to lower floor
    if(ps2x.ButtonPressed(PSB_R2)){
		//Serial.println("R2 Pressed");
		motorA.spin(0);
		motorB.spin(0);
		motorC.spin(0);
		toLow();
		prev_button = PSB_R2;
    }
   
	// go to upper floor
	if(ps2x.ButtonPressed(PSB_L2)){
		//Serial.println("L2 PRESSED");
		motorA.spin(0);
		motorB.spin(0);
		motorC.spin(0);
		toHigh();
		prev_button = PSB_L2;
	}
	
	// arm up
	if(ps2x.ButtonReleased(PSB_PAD_UP)){
	  //Serial.println("UP released");
	}
	else if(ps2x.Button(PSB_PAD_UP)){
		if(udpos >= 25){
			if(!(prev_button == PSB_PAD_UP || prev_button == PSB_PAD_DOWN)){
				arm_x = getArm_x(udpos, bfpos);	// refresh arm_x
			}
			udpos -= increment;
			updown.write(udpos);
			
			bfpos = getBackForwardAngle(arm_x, udpos, bfpos);	// inverse kinematic
			backforward.write(bfpos);
			
			prev_button = PSB_PAD_UP;
			
			/*Serial.print("up-down: ");
			Serial.print(udpos);
			Serial.print(", back-forward: ");
			Serial.println(bfpos);*/
			
			/*if(inLow && udpos >= 45 && udpos <= 75 && bfpos >= 10 && bfpos <= 30){
				bfpos = (int)(low_back_forward[udpos-45]+0.5);
				backforward.write(bfpos);
				Serial.print("back-forward: ");
				Serial.println(bfpos);
			}
			else if(inHigh && udpos >= 25 && udpos <= 45){
				bfpos = (int)(high_back_forward[udpos-25]+0.5);
				backforward.write(bfpos);
				Serial.print("back-forward: ");
				Serial.println(bfpos);
			}*/
			delay(10);
		}
		else{
			//Serial.println("up-down position reaches lower limit");
		}
	}

	// arm down
	if(ps2x.ButtonReleased(PSB_PAD_DOWN)){}
	else if(ps2x.Button(PSB_PAD_DOWN)){
		if(udpos <= 150){
			if(!(prev_button == PSB_PAD_UP || prev_button == PSB_PAD_DOWN)){
				arm_x = getArm_x(udpos, bfpos);	// refresh arm_x
			}
			udpos += increment;
			updown.write(udpos);
			
			bfpos = getBackForwardAngle(arm_x, udpos, bfpos);	// inverse kinematic
			backforward.write(bfpos);
			
			/*Serial.print("up-down: ");
			Serial.print(udpos);
			Serial.print(", back-forward: ");
			Serial.println(bfpos);*/
			
			prev_button = PSB_PAD_DOWN;
			
			/*if(inLow && udpos >= 45 && udpos <= 75 && bfpos >= 10 && bfpos <= 30){
				bfpos = (int)(low_back_forward[udpos-45]+0.5);
				backforward.write(bfpos);
				Serial.print("back-forward: ");
				Serial.println(bfpos);
			}
			else if(inHigh && udpos >= 25 && udpos <= 45){
				bfpos = (int)(high_back_forward[udpos-25]+0.5);
				backforward.write(bfpos);
				Serial.print("back-forward: ");
				Serial.println(bfpos);
			}*/
			delay(10);
		}
		else{
			//Serial.println("up-down position reaches upper limit");
		}
	}
    
	// arm forward
	if(ps2x.ButtonReleased(PSB_PAD_LEFT)){}
	else if(ps2x.Button(PSB_PAD_LEFT)){
		if(bfpos <= 110){
			bfpos += increment;
			backforward.write(bfpos);
			Serial.print("back-forward: ");
			Serial.println(bfpos);
			
			prev_button = PSB_PAD_LEFT;
			
			/*if(inLow && bfpos >= 10 && bfpos <= 30 && udpos >= 45 && udpos <= 75){
				udpos = (int)(low_up_down[bfpos-10]+0.5);
				updown.write(udpos);
				Serial.print("up-down: ");
				Serial.println(udpos);
			}
			//else if(inHigh){}*/
			delay(10);
		}
		else{
			//Serial.println("back-forward position reaches upper limit");
		}
	}

	// arm back
	if(ps2x.ButtonReleased(PSB_PAD_RIGHT)){}
	else if(ps2x.Button(PSB_PAD_RIGHT)){ 
		if(bfpos >= 10){
			bfpos -= increment;
			backforward.write(bfpos);
			Serial.print("back-forward: ");
			Serial.println(bfpos);
			
			prev_button = PSB_PAD_RIGHT;
			
			/*if(inLow && bfpos >= 10 && bfpos <= 30 && udpos >= 45 && udpos <= 75){
				udpos = (int)(low_up_down[bfpos-10]+0.5);
				updown.write(udpos);
				Serial.print("updown: ");
				Serial.println(udpos);
			}
			//else if(inHigh){}*/
			delay(10);
		}
		else{
			//Serial.println("back-forward position reaches lower limit");
		}
	}
}

/*void conveyorControl(){
	/*static int inSize;
	static char c;
	static byte conveyor_mode = STOP;
	
	if((inSize = BT.available()) > 0){
		c = (char)BT.read();
	}
	
	if(c == 'A')
		conveyor_mode = UP;
	else if(c == 'B')
		conveyor_mode = DOWN;
	else if(c == 'q')
		conveyor_mode = STOP;
	
	if(conveyor_mode == UP){
		Serial.println("Conveyor going up");
		conveyor.spin(255, CW);
	}
	else if(conveyor_mode == DOWN){
		Serial.println("Conveyor going down");
		conveyor.spin(255, CCW);
	}
	else{
		conveyor.spin(0);
	}
	
	// downward
    if(ps2x.Button(PSB_L1)){
		//Serial.println("L1 pressed: Conveyor goes downward");
		conveyor.spin(255, CW);
    }
    // upward
    else if(ps2x.Button(PSB_R1)){
		//Serial.println("R1 pressed: Conveyor goes upward");
		conveyor.spin(255, CCW);
    }
	else{
		conveyor.spin(0);
	}
}*/

/*void electromagnetControl(){
	if(ps2x.ButtonPressed(PSB_L2)){
		digitalWrite(ELECTROMAGNET, HIGH);
    }
    else if(ps2x.ButtonPressed(PSB_R2)){
		digitalWrite(ELECTROMAGNET, LOW);
    }
}*/

void toMiddle(){
	//inHigh = false;
	//inLow = false;
	increment = 2;
	
	//Serial.println("to Middle");
	int _increment = 0;
	
	_increment = bfMiddle - bfpos > 0 ? 1 : -1;
	for(int i = bfpos; i != bfMiddle; i += _increment){
		backforward.write(i);
		//Serial.println(i);
		delay(10);  
	}
	bfpos = bfMiddle;
	
	_increment = udMiddle - udpos > 0 ? 1 : -1;
	for(int i = udpos; i != udMiddle; i += _increment){
		updown.write(i);
		//Serial.println(i);
		delay(10);
	}
	udpos = udMiddle;                                                                                                                                                                                                                                                                                                  
	//Serial.println("OK");
}

void toHigh(){
	//inHigh = true;
	//inLow = false;
	increment = 1;
	
	//Serial.println("to High");
	int _increment = 0;

	_increment = (udHigh - udpos > 0) ? 1 : -1;
	for(int i = udpos; i != udHigh; i += _increment){
		updown.write(i);
		//Serial.println(i);
		delay(10);
	}
	udpos = udHigh;
                                                                                                                                                                                                                                                                                                           
	_increment = (bfHigh - bfpos > 0) ? 1 : -1;
	for(int i = bfpos; i != bfHigh; i += _increment){
		backforward.write(i);
		//Serial.println(i);
		delay(10);  
	}
	bfpos = bfHigh;
	//Serial.println("OK");
}

void toLow(){
	//inLow = true;
	//inHigh = false;
	increment = 1;
	
	//Serial.println("to Low");
	int _increment = 0;

	_increment = (udLow - udpos > 0) ? 1 : -1;
	for(int i = udpos; i != udLow; i += _increment){
		//ps2x.read_gamepad(false, vibrate);
		updown.write(i);
		//Serial.println(i);
		delay(10);
	}
	udpos = udLow;
                                                                                                                                                                                                                                                                                                           
	_increment = (bfLow - bfpos > 0) ? 1 : -1;
	for(int i = bfpos; i != bfLow; i += _increment){
		backforward.write(i);
		//Serial.println(i);
		delay(10);
	}
	bfpos = bfLow;
	//Serial.println("OK");
}

/*void setAngle(int udpos_new, int bfpos_new){
    if(finished){
		finished = false;
		int _increment = 0;
	
		_increment = udpos_new - udpos > 0 ? 1 : -1;
		for(int i = udpos; i != udpos_new; i += _increment){
			updown.write(i);
			Serial.println(i);
			delay(15);
		}
		udpos = udpos_new;
                                                                                                                                                                                                                                                                                                                    
		_increment = bfpos_new - bfpos > 0 ? 1 : -1;
		for(int i = bfpos; i != bfpos_new; i += _increment){
			backforward.write(i);
			Serial.println(i);
			delay(15);  
		}
		bfpos = bfpos_new;
		Serial.println("OK");
		finished = true;
	}
}*/

void carMovement(){
	x = ps2x.Analog(PSS_LX);
	y = ps2x.Analog(PSS_LY);
	
	/*if(!(x == 128 && y == 127)){
		Serial.print("x: ");
		Serial.print(x);
		Serial.print(", y: ");
		Serial.println(y);
	}*/
	
	// prevent impulse
	if(x == prev_x && y == prev_y && x == prev_prev_x && y == prev_prev_y){
		// left-forward
		if(x < 70 && y < 70){
			//Serial.println("Left-Forward");
			motorA.spin(fullspeed, CW);
			motorB.spin(fullspeed, CCW);
			motorC.spin(0);
		}
		
		// right-forward
		else if(x > 184 && y < 70){
			//Serial.println("Right-Forward");
			motorA.spin(0);
			motorB.spin(fullspeed, CCW);
			motorC.spin(fullspeed, CW);
		}
		
		// right-backward
		else if(x > 184 && y > 184){
			//Serial.println("Right-Backward");
			motorA.spin(fullspeed, CCW);
			motorB.spin(fullspeed, CW);
			motorC.spin(0);
		}
		
		// left-backward
		else if(x < 70 && y > 184){
			//Serial.println("Left-Backward");
			motorA.spin(0);
			motorB.spin(fullspeed, CW);
			motorC.spin(fullspeed, CCW);
		}
		
		// forward
		else if(x > 70 && x < 184 && y < 70){
			//Serial.println("Forward");
			motorA.spin(halfspeed, CW);
			motorB.spin(fullspeed, CCW);
			motorC.spin(halfspeed, CW);
		}
		
		// right
		else if(x > 184 && y > 70 && y < 184){
			//Serial.println("Right");
			motorA.spin(fullspeed, CCW);
			motorB.spin(0);
			motorC.spin(fullspeed, CW);
		}
		
		// backward
		else if(x > 70 && x < 184 && y > 184){
			//Serial.println("Backward");
			motorA.spin(halfspeed, CCW);
			motorB.spin(fullspeed, CW);
			motorC.spin(halfspeed, CCW);
		}
		
		// left
		else if(x < 70 && y > 70 && y < 184){
			//Serial.println("Left");
			motorA.spin(fullspeed, CW);
			motorB.spin(0);
			motorC.spin(fullspeed, CCW);
		}
		
		// stop
		else{
			motorA.spin(0);
			motorB.spin(0);
			motorC.spin(0);
		}
	}
	else{	// refresh prev_x & prev_y when arduino detect different value
		prev_prev_x = prev_x;
		prev_prev_y = prev_y;
		prev_x = x;
		prev_y = y;
	}
}

void rotate(){
	x = ps2x.Analog(PSS_RX);
	y = ps2x.Analog(PSS_RY);
	byte LX = ps2x.Analog(PSS_LX);
	byte LY = ps2x.Analog(PSS_LY);
	
	// prevent impulse
	if(x == 255 && y == 255) return;
	
	if(mode == FAST){
    /*Serial.print("x: ");
    Serial.print(x);
    Serial.print(", y: ");
    Serial.println(y);*/
    
		if(x < 70){
			//Serial.println("Wheel rotate CW; Car rotate CCW");
			motorA.spin(70, CW);
			motorB.spin(70, CW);
			motorC.spin(70, CW);
		}
		else if(x > 184){
			//Serial.println("Wheel rotate CCW; Car rotate CW");
			motorA.spin(70, CCW);
			motorB.spin(70, CCW);
			motorC.spin(70, CCW);
		}
		else if(LX > 70 && LX < 184 && LY > 70 && LY < 184){
			motorA.spin(0);
			motorB.spin(0);
			motorC.spin(0);
		}
	}
	else if(mode == SLOW){
		if(x < 70){
			Serial.println("Wheel rotate CW; Car rotate CCW");
			motorA.spin(35, CW);
			motorB.spin(35, CW);
			motorC.spin(35, CW);
		}
		else if(x > 184){
			Serial.println("Wheel rotate CCW; Car rotate CW");
			motorA.spin(35, CCW);
			motorB.spin(35, CCW);
			motorC.spin(35, CCW);
		}
		else if(LX > 70 && LX < 184 && LY > 70 && LY < 184){
			motorA.spin(0);
			motorB.spin(0);
			motorC.spin(0);
		}
	}
}
