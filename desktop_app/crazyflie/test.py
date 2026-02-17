import sys
import time

import cflib.crtp
from cflib.crazyflie import Crazyflie
from cflib.crazyflie.log import LogConfig
from cflib.crazyflie.syncCrazyflie import SyncCrazyflie
from cflib.utils import uri_helper

# Change to your URI
uri = uri_helper.uri_from_env(default="udp://192.168.43.42:2390")

HOVER_THRUST = 8000
RAMP_TIME = 2.0


def run_hover(cf):
    print("Ramping up thrust...")

    # Gradually increase thrust to avoid motor shock
    steps = 50
    for i in range(steps):
        thrust = int(HOVER_THRUST * (i / steps))
        cf.commander.send_setpoint(0, 0, 0, thrust)
        time.sleep(RAMP_TIME / steps)

    while 1:
        cf.commander.send_setpoint(0, 0, 0, HOVER_THRUST)


def log_callback(timestamp, data, logconf):
    print(
        f"{timestamp} | "
        f"roll: {data['stabilizer.roll']:.2f}, "
        f"pitch: {data['stabilizer.pitch']:.2f}, "
        f"thrust: {data['stabilizer.thrust']:.2f}"
    )


if __name__ == "__main__":
    cflib.crtp.init_drivers()

    with SyncCrazyflie(uri, cf=Crazyflie(rw_cache="./cache")) as scf:
        cf = scf.cf
        log_config = LogConfig(name="Stab", period_in_ms=100)
        log_config.add_variable("stabilizer.roll", "float")
        log_config.add_variable("stabilizer.pitch", "float")
        log_config.add_variable("stabilizer.thrust", "float")
        cf.log.add_config(log_config)

        if not log_config.valid:
            print("Log configuration invalid")
            sys.exit(1)

        log_config.data_received_cb.add_callback(log_callback)
        log_config.start()

        run_hover(cf)

    print("Done.")
