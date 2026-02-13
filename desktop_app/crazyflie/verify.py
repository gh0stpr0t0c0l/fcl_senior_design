import time

import cflib.crtp
from cflib.crazyflie import Crazyflie
from cflib.crazyflie.log import LogConfig
from cflib.crazyflie.syncCrazyflie import SyncCrazyflie

URI = "udp://192.168.43.42:2390"


def log_callback(timestamp, data, logconf):
    print(f"[{timestamp}] Battery: {data['pm.vbat']:.2f} V")


if __name__ == "__main__":
    cflib.crtp.init_drivers()

    with SyncCrazyflie(URI, cf=Crazyflie(rw_cache="./cache")) as scf:
        logconf = LogConfig(name="Battery", period_in_ms=500)

        logconf.add_variable("pm.vbat", "float")

        scf.cf.log.add_config(logconf)
        logconf.data_received_cb.add_callback(log_callback)

        logconf.start()
        time.sleep(10)
        logconf.stop()
