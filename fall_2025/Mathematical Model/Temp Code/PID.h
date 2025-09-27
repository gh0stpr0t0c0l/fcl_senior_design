/*#pragma once

using namespace std;

class PID {
public:
	int P;
	int I;
	int D;

	PID() {
		P = 0;
		I = 0;
		D = 0;
	}

	PID(int prop, int integ, int deriv) {
		P = prop;
		I = integ;
		D = deriv;
	}

	void updateVals(float newe) {
		//update array
		for (int i = 0; i < 20; i++) {
			e[i] = e[i + 1];
		}
		//update sum
		e[49] = newe;
		sum += newe;
		if (sum > 100) {
			sum = 100;
		}
		//update slope (Taking 2 sets of 10 terms)
		//Refer to slideshow for math
		slope = slope + (newe + e[0] - 2 * e[10]) / 2;
	}

	int getDutyCycle() {
		int DC = P * e[20] + I * sum - D * slope;
		if (DC > 100) {
			return 100;
		}
		return DC;
	}

private:
	float e[21] = { 0 };
	float sum = 0;
	float slope = 0;
};*/

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
	cont.e[1] = 1;
	cont.sum = 0;
	cont.slope = 0;
	return cont;
}

void PID_updateVals(PID* cont, float newe) {
	cont->e[0] = cont->e[1];
	cont->e[1] = newe;
	cont->sum += newe;
	if (cont->sum > 100) {//to prevent integral overshoot
		cont->sum = 100;
	}
	cont->slope = cont->e[1] - cont->e[0];
}

int getDutyCycle(PID* cont) {
	int DC = cont->P * cont->e[1] + cont->I * cont->sum - cont->D * cont->slope;
	if (DC > 100) {
		return 100;
	}
	return DC;
}