import sys
import time

import cflib.crtp
from cflib.crazyflie import Crazyflie
from cflib.crazyflie.log import LogConfig
from cflib.crazyflie.syncCrazyflie import SyncCrazyflie
from cflib.utils import uri_helper

# Change if needed
uri = uri_helper.uri_from_env(default="udp://192.168.43.42:2390")

HOVER_THRUST = 30000  # Adjust as needed
COMMAND_RATE_HZ = 50  # Setpoint update rate
LOG_RATE_MS = 50  # 20 ms = 50 Hz logging
MOTOR_LOG = 1
JUST_LOG = 0


def log_callback(timestamp, data, logconf):
    if MOTOR_LOG:
        print(
            f"{timestamp} | "
            f"MOTOR: "
            f"{data['motor.m1']:5d} "
            f"{data['motor.m2']:5d} "
            f"{data['motor.m3']:5d} "
            f"{data['motor.m4']:5d}   |   "
            f"PWM: "
            f"{data['pwm.m1_pwm']:5d} "
            f"{data['pwm.m2_pwm']:5d} "
            f"{data['pwm.m3_pwm']:5d} "
            f"{data['pwm.m4_pwm']:5d}"
        )
    else:
        print(
            f"{timestamp} | "
            f"roll: {data['stabilizer.roll']:.2f}, "
            f"pitch: {data['stabilizer.pitch']:.2f}, "
            f"thrust: {data['stabilizer.thrust']:.2f}"
        )


def run_spin_test(cf):
    print("Motors spinning. Press Ctrl+C to stop.")

    dt = 1.0 / COMMAND_RATE_HZ

    for _ in range(20):
        cf.commander.send_setpoint(0, 0, 0, 0)
        time.sleep(0.02)

    try:
        while True:
            # zero roll, pitch, yaw-rate, constant thrust
            if JUST_LOG:
                cf.commander.send_setpoint(0, 0, 0, 0)
            else:
                cf.commander.send_setpoint(0, 0, 0, HOVER_THRUST)
            time.sleep(dt)

    except KeyboardInterrupt:
        print("\nStopping motors...")
        cf.commander.send_setpoint(0, 0, 0, 0)
        time.sleep(0.1)


if __name__ == "__main__":
    cflib.crtp.init_drivers()

    with SyncCrazyflie(uri, cf=Crazyflie(rw_cache="./cache")) as scf:
        cf = scf.cf

        # ---- Logging Setup ----
        log_config = LogConfig(name="Stab", period_in_ms=LOG_RATE_MS)
        if not MOTOR_LOG:
            log_config.add_variable("stabilizer.roll", "float")
            log_config.add_variable("stabilizer.pitch", "float")
            # log_config.add_variable("stabalizer.yaw, float")
            log_config.add_variable("stabilizer.thrust", "float")
        else:
            log_config.add_variable("motor.m1", "uint16_t")
            log_config.add_variable("motor.m2", "uint16_t")
            log_config.add_variable("motor.m3", "uint16_t")
            log_config.add_variable("motor.m4", "uint16_t")
            log_config.add_variable("pwm.m1_pwm", "uint16_t")
            log_config.add_variable("pwm.m2_pwm", "uint16_t")
            log_config.add_variable("pwm.m3_pwm", "uint16_t")
            log_config.add_variable("pwm.m4_pwm", "uint16_t")

        cf.log.add_config(log_config)

        if not log_config.valid:
            print("Log configuration invalid")
            sys.exit(1)

        log_config.data_received_cb.add_callback(log_callback)
        log_config.start()

        # ---- Run motors continuously ----
        run_spin_test(cf)

        # Stop logging before exit
        log_config.stop()

    print("Disconnected cleanly.")
