#pragma once

#include "PWM.h"
#include "PID.h"

//2 NOTES:  this file (presently) assumes that the desired pitch and roll have been found from the xy positional PIDs
//          the PIDs should be allocated and deallocated outside of this function. 
void runPID(int errors[4]) {
    const double A = 0.7;
    const double B = 0.1;
    int dc[4] = 0;          // the duty cycles obtained from the last PID controllers
    int pwm[4] = 0;         // the final pwm signal used

    //Make PID controllers. This should actually be done outside of this function. probably. maybe. perchance. methinks.
    struct PID* thrust = (struct PID*)malloc(sizeof(struct PID));
    struct PID* yaw = (struct PID*)malloc(sizeof(struct PID));
    struct PID* pitch = (struct PID*)malloc(sizeof(struct PID));
    struct PID* roll = (struct PID*)malloc(sizeof(struct PID));

    thrust* = PID_init(10, 0.1, 0.1);
    yaw* = PID_init(10, 0.1, 0.1);
    pitch* = PID_init(10, 0.1, 0.1);
    roll* = PID_init(10, 0.1, 0.1);

    PID_updateVals(thrust, errors[0]);
    PID_updateVals(yaw, errors[1]);
    PID_updateVals(pitch, errors[2]);
    PID_updateVals(roll, errors[3]);

    int thrustDC = getDutyCycle(thrust);
    int yawDC = getDutyCycle(yaw);
    int pitchDC = getDutyCycle(pitch);
    int rollDC = getDutyCycle(roll);


    pwm[0] = A * thrustDC + B * yawDC + B * pitchDC + B * rollDC;
    pwm[1] = A * thrustDC - B * yawDC + B * pitchDC - B * rollDC;
    pwm[2] = A * thrustDC - B * yawDC - B * pitchDC + B * rollDC;
    pwm[3] = A * thrustDC + B * yawDC - B * pitchDC - B * rollDC;

    for (int i = 0; i < 4; i++) {
        ledc_set_duty(SPEED_MODE, i, pwm[i]);   //sets the new value
        ledc_update_duty(SPEED_MODE, i);    //applies the new value
    }

    //deallocate PIDs
    free(thrust);
    free(yaw);
    free(pitch);
    free(roll);
}