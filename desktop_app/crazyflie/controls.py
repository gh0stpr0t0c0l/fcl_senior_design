import sys
import time

import cflib.crtp
from cflib.crazyflie import Crazyflie
from cflib.crazyflie.syncCrazyflie import SyncCrazyflie
from cflib.utils import uri_helper

# Change to your URI
uri = uri_helper.uri_from_env(default="udp://192.168.43.42:2390")

HOVER_THRUST = 1000  # You may need to tune this
RAMP_TIME = 2.0  # seconds
HOVER_TIME = 15.0  # seconds


def run_hover(cf):
    print("Ramping up thrust...")

    # Gradually increase thrust to avoid motor shock
    steps = 50
    for i in range(steps):
        thrust = int(HOVER_THRUST * (i / steps))
        cf.commander.send_setpoint(0, 0, 0, thrust)
        time.sleep(RAMP_TIME / steps)

    print("Holding hover attitude...")
    start = time.time()

    while time.time() - start < HOVER_TIME:
        # 0 roll, 0 pitch, 0 yaw rate
        cf.commander.send_setpoint(0, 0, 0, HOVER_THRUST)
        time.sleep(0.02)  # 50 Hz command rate

    print("Stopping motors...")
    cf.commander.send_setpoint(0, 0, 0, 0)
    time.sleep(0.5)
    cf.commander.send_stop_setpoint()


if __name__ == "__main__":
    cflib.crtp.init_drivers()

    with SyncCrazyflie(uri, cf=Crazyflie(rw_cache="./cache")) as scf:
        run_hover(scf.cf)

    print("Done.")
