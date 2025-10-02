#pragma once
/*#pragma once

using namespace std;

@@ -20,7 +20,7 @@ class PID {
		D = deriv;
	}

	int updateVals(float newe) {
	void updateVals(float newe) {
		//update array
		for (int i = 0; i < 20; i++) {
			e[i] = e[i + 1];
@@ -48,4 +48,47 @@ class PID {
	float e[21] = { 0 };
	float sum = 0;
	float slope = 0;
};*/

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
	if (cont->sum > 255) {//to prevent integral overshoot
		cont->sum = 255;
	}
	cont->slope = cont->e[1] - cont->e[0];
}

int getDutyCycle(PID* cont) {
	int DC = cont->P * cont->e[1] + cont->I * cont->sum - cont->D * cont->slope;
	if (DC > 255) {
		return 255;
	}
	return DC;
}
