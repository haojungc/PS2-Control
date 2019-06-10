#ifndef DCMOTOR_H
#define DCMOTOR_H

#define CW true
#define CCW false

class MOTOR{
	public:
		MOTOR();
		MOTOR(int, int);
		void setPin(int, int);
		void setSpeed(int);
		void setDirection(bool);
		void spin();
		void spin(int);
		void spin(int, bool);
		static const int fullspeed = 128;
		static const int halfspeed = 64;
	
	private:
		int speed;
		bool direction;	// CW or CCW
		int pin1, pin2;
};

#endif
