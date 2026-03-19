import sys
import time

import cflib.crtp
from cflib.crazyflie import Crazyflie
from cflib.crazyflie.log import LogConfig
from cflib.crazyflie.syncCrazyflie import SyncCrazyflie
from cflib.utils import uri_helper

# Change if needed
uri = uri_helper.uri_from_env(default="udp://192.168.43.42:2390")

HOVER_THRUST = 25000  # Adjust as needed
COMMAND_RATE_HZ = 50  # Setpoint update rate
LOG_RATE_MS = 50  # 20 ms = 50 Hz logging
LOG_TYPE = 1  # 0=motors; 1=Pitch PIDs; 2=stateEstimator; 3=motor pwms; 3=Roll PIDs
JUST_LOG = 0
FLIGHT_TYPE = 0  # 0=gimbal; 1=altitude


def log_callback(timestamp, data, logconf):
    with open("flightDump.csv", "a") as log:
        log.write(
            f"{timestamp},",
        )
        if LOG_TYPE == 0:
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
            log.write(
                f"{data['motor.m1']:5d},"
                f"{data['motor.m2']:5d},"
                f"{data['motor.m3']:5d},"
                f"{data['motor.m4']:5d},"
                f"{data['pwm.m1_pwm']:5d},"
                f"{data['pwm.m2_pwm']:5d},"
                f"{data['pwm.m3_pwm']:5d},"
                f"{data['pwm.m4_pwm']:5d}\n"
            )

        elif LOG_TYPE == 1:
            print(
                f"{timestamp} | ,"
                # f"roll: {data['stabilizer.roll']:.2f}, "
                # f"pitch: {data['stabilizer.pitch']:.2f}, "
                # f"yaw: {data['stabilizer.yaw']:.2f}, "
                #f"ROLL: "
                #f"PRoll: {data['pid_rate.roll_outP']:.2f}, "
                #f"IRoll: {data['pid_rate.roll_outI']:.2f}, "
                #f"DRoll: {data['pid_rate.roll_outD']:.2f}, "
                f"PPitch: {data['pid_attitude.pitch_outP']:.2f}, "
                f"IPitch: {data['pid_attitude.pitch_outI']:.2f}, "
                f"DPitch: {data['pid_attitude.pitch_outD']:.2f}, "
                f"PPitch: {data['pid_rate.pitch_outP']:.2f}, "
                f"IPitch: {data['pid_rate.pitch_outI']:.2f}, "
                f"DPitch: {data['pid_rate.pitch_outD']:.2f}, "
                #f"Roll: {data['stabilizer.roll']:.2f}, "
                #f"Pitch: {data['stabilizer.pitch']:.2f}, "
                # f"thrust: {data['stabilizer.thrust']:.2f}"
            )
            log.write(
                # f"roll: {data['stabilizer.roll']:.2f}, "
                # f"pitch: {data['stabilizer.pitch']:.2f}, "
                # f"yaw: {data['stabilizer.yaw']:.2f}, "
                #f"{data['pid_rate.roll_outP']:.2f},"
                #f"{data['pid_rate.roll_outI']:.2f},"
                #f"{data['pid_rate.roll_outD']:.2f},"
                f"{data['pid_attitude.pitch_outP']:.2f}, "
                f"{data['pid_attitude.pitch_outI']:.2f}, "
                f"{data['pid_attitude.pitch_outD']:.2f}, "
                f"{data['pid_rate.pitch_outP']:.2f},"
                f"{data['pid_rate.pitch_outI']:.2f},"
                f"{data['pid_rate.pitch_outD']:.2f}\n"
                #f"{data['stabilizer.roll']:.2f},\n"
                #f"{data['stabilizer.pitch']:.2f}\n"
                # f"thrust: {data['stabilizer.thrust']:.2f}"
            )

        elif LOG_TYPE == 3:
            print(
                f"{timestamp} | ,"
                f"PPitch: {data['pid_attitude.roll_outP']:.2f}, "
                f"IPitch: {data['pid_attitude.roll_outI']:.2f}, "
                f"DPitch: {data['pid_attitude.roll_outD']:.2f}, "
                f"PPitch: {data['pid_rate.roll_outP']:.2f}, "
                f"IPitch: {data['pid_rate.roll_outI']:.2f}, "
                f"DPitch: {data['pid_rate.roll_outD']:.2f}, "
            )
            log.write(
                f"{data['pid_attitude.roll_outP']:.2f}, "
                f"{data['pid_attitude.roll_outI']:.2f}, "
                f"{data['pid_attitude.roll_outD']:.2f}, "
                f"{data['pid_rate.roll_outP']:.2f},"
                f"{data['pid_rate.roll_outI']:.2f},"
                f"{data['pid_rate.roll_outD']:.2f}\n"
            )

        elif LOG_TYPE == 2:
            print(
                f"{timestamp} | "
                f"State Estimate: "
                f"X: {data['stateEstimate.x']:.2f}, "
                f"Y: {data['stateEstimate.y']:.2f}, "
                f"Z: {data['stateEstimate.z']:.2f}"
            )
            log.write(
                f"{data['stateEstimate.x']:.2f},"
                f"{data['stateEstimate.y']:.2f},"
                f"{data['stateEstimate.z']:.2f}\n"
            )



def flight_test(cf):
    print("Starting. Press Ctrl+C to stop.")

    dt = 1.0 / COMMAND_RATE_HZ

    for _ in range(20):
        cf.commander.send_setpoint(0, 0, 0, 0)
        time.sleep(0.02)

    if FLIGHT_TYPE == 0:
        try:
            while True:
                # zero roll, pitch, yaw-rate, constant thrust
                if JUST_LOG:
                    cf.commander.send_setpoint(0, 0, 0, 0)
                else:
                    cf.commander.send_setpoint(0, 0, 0, HOVER_THRUST)
                time.sleep(dt)

        except KeyboardInterrupt:
            print("\nStopping...")
            cf.commander.send_setpoint(0, 0, 0, 0)
            time.sleep(0.1)

    elif FLIGHT_TYPE == 1:
        try:
            while True:
                if JUST_LOG:
                    cf.commander.send_setpoint(0, 0, 0, 0)
                else:
                    cf.commander.send_position_setpoint(0.0, 0.0, 0.3, 0.0)
                time.sleep(dt)

        except KeyboardInterrupt:
            print("\nStopping...")
            cf.commander.send_setpoint(0, 0, 0, 0)
            time.sleep(0.1)

        # elif FLIGHT_TYPE == 1:
    #     try:
    #         hlc = cf.high_level_commander
    #         time.sleep(1)
    #         hlc.takeoff(0.3, 2.0)
    #         time.sleep(5)
    #         hlc.land(0.0, 2.0)
    #         time.sleep(2)
    #         hlc.stop()
    #         print("Done")

    #     except KeyboardInterrupt:
    #         print("\nStopping...")
    #         cf.commander.send_setpoint(0, 0, 0, 0)
    #         time.sleep(0.1)


if __name__ == "__main__":
    cflib.crtp.init_drivers()

    with SyncCrazyflie(uri, cf=Crazyflie(rw_cache="./cache")) as scf:
        cf = scf.cf

        # ---- Logging Setup ----
        log_config = LogConfig(name="Stab", period_in_ms=LOG_RATE_MS)

        with open("flightDump.csv", "w") as f:
            f.write("Time(us),")
            if LOG_TYPE == 1:
                # log_config.add_variable("stabilizer.roll", "float")
                # log_config.add_variable("stabilizer.pitch", "float")
                # log_config.add_variable("stabilizer.yaw", "float")
                #log_config.add_variable("pid_rate.roll_outP", "float")
                #log_config.add_variable("pid_rate.roll_outI", "float")
                #log_config.add_variable("pid_rate.roll_outD", "float")
                log_config.add_variable("pid_attitude.pitch_outP", "float")
                log_config.add_variable("pid_attitude.pitch_outI", "float")
                log_config.add_variable("pid_attitude.pitch_outD", "float")
                log_config.add_variable("pid_rate.pitch_outP", "float")
                log_config.add_variable("pid_rate.pitch_outI", "float")
                log_config.add_variable("pid_rate.pitch_outD", "float")
                #log_config.add_variable("stabilizer.roll", "float")
                #log_config.add_variable("stabilizer.pitch", "float")

                f.write(
                    "Pitch P,Pitch I,Pitch D,Pitch P rate,Pitch I rate,Pitch D rate\n"
                )
                # log_config.add_variable("stabilizer.thrust", "float")
            elif LOG_TYPE == 3:
                log_config.add_variable("pid_attitude.roll_outP", "float")
                log_config.add_variable("pid_attitude.roll_outI", "float")
                log_config.add_variable("pid_attitude.roll_outD", "float")
                log_config.add_variable("pid_rate.roll_outP", "float")
                log_config.add_variable("pid_rate.roll_outI", "float")
                log_config.add_variable("pid_rate.roll_outD", "float")

                f.write(
                    "Roll P,Roll I,Roll D,Roll P rate,Roll I rate,Roll D rate\n"
                )
                # log_config.add_variable("stabilizer.thrust", "float")
            elif LOG_TYPE == 0:
                log_config.add_variable("motor.m1", "uint16_t")
                log_config.add_variable("motor.m2", "uint16_t")
                log_config.add_variable("motor.m3", "uint16_t")
                log_config.add_variable("motor.m4", "uint16_t")
                log_config.add_variable("pwm.m1_pwm", "uint16_t")
                log_config.add_variable("pwm.m2_pwm", "uint16_t")
                log_config.add_variable("pwm.m3_pwm", "uint16_t")
                log_config.add_variable("pwm.m4_pwm", "uint16_t")

                f.write("m1,m2,m3,m4,pwm1,pwm2,pwm3,pwm4\n")
            elif LOG_TYPE == 2:
                log_config.add_variable("stateEstimate.x", "float")
                log_config.add_variable("stateEstimate.y", "float")
                log_config.add_variable("stateEstimate.z", "float")

                f.write("x estimate,y estimate,z estimate\n")

        cf.log.add_config(log_config)

        if not log_config.valid:
            print("Log configuration invalid")
            sys.exit(1)

        log_config.data_received_cb.add_callback(log_callback)
        log_config.start()

        # ---- Run motors continuously ----
        flight_test(cf)

        # Stop logging before exit
        log_config.stop()

    print("Disconnected cleanly.")
