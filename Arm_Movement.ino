#include "Arm_Movement.h"

double getArm_x(int theta_ud, int theta_bf){
	double theta1 = (double)(theta_bf)*DEG_TO_RAD;
	double theta2 = (double)(240-theta_ud)*DEG_TO_RAD;
	double x = 148*cos(theta1) - 159.25*cos(theta2);

	return x;
}

int getBackForwardAngle(double x, int theta_ud, int theta_bf){
	double segment = (x+159.25*cos((double)(240-theta_ud)*DEG_TO_RAD))/148;

	// make sure segment is in acos range
	if(segment > 1.0 || segment < -1.0)
		return theta_bf;

	int new_theta_bf = (int)(acos(segment)*RAD_TO_DEG + 0.5);

	if(new_theta_bf >= 10 || new_theta_bf <= 110)
		return new_theta_bf;
	else
		return theta_bf;
}
