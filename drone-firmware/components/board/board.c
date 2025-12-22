#include "board.h"
// #include "include/board.h"
#include <stdint.h>

// Declare tof xshut pins
// const uint8_t tof_xshut_pins[TOF_COUNT] = { GPIO_NUM_18, GPIO_NUM_19 };
const uint8_t tof_xshut_pins[TOF_COUNT] = {
    TOF_XSHUT_PIN_0, TOF_XSHUT_PIN_1
};
// const uint8_t tof_xshut_pins[TOF_COUNT] = {
//     TOP_XSHUT_PIN, INNER_XSHUT_PIN, SINGLE_XSHUT_PIN, OUTER_XSHUT_PIN, BOT_XSHUT_PIN
// };

// Declare motor pwm pins
// const uint8_t motor_pins[MOTOR_COUNT] = {
//     MOTOR_1_PIN, MOTOR_2_PIN, MOTOR_3_PIN, MOTOR_4_PIN
// };

const uint8_t *board_get_tof_pins(void) { return tof_xshut_pins; }
