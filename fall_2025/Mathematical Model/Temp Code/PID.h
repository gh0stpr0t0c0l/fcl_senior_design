#pragma once

using namespace std;

struct PID {
	int P;
	int I;
	int D;
	float e[2];
	float sum;
	float slope;
};

PID PID_init(int prop, int integ, int deriv) {
	PID cont;
	cont.P = prop;
	cont.I = integ;
	cont.D = deriv;
	cont.e[0] = 0;
	cont.e[1] = 0;
	cont.sum = 0;
	cont.slope = 0;
	return cont;
}

void PID_updateVals(PID* cont, float newe) {
	cont->e[0] = cont->e[1];
	cont->e[1] = newe;
	cont->sum += newe;
	if (cont->sum > 128) {//to prevent integral overshoot
		cont->sum = 128;
	}
	else if (cont->sum < -127) {
		cont->sum = -127;
	}
	cont->slope = cont->e[1] - cont->e[0];
}

int getDutyCycle(PID* cont) {
	int DC = (int)(cont->P * cont->e[1] + cont->I * cont->sum - cont->D * cont->slope);
	DC += 127;//Get into the 0-255 range
	if (DC > 255) {
		return 255;
	}
	else if (DC < 0) {
		return 0;
	}
	return DC;
}
