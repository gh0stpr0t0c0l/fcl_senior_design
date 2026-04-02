import sys
import time

import cflib.crtp
from cflib.crazyflie import Crazyflie
from cflib.crazyflie.log import LogConfig
from cflib.crazyflie.syncCrazyflie import SyncCrazyflie

URI = "udp://192.168.43.42:2390"
LOG_RATE_MS = 20  # 20 ms = 50 Hz logging


def log_callback(timestamp, data, logconf):
    print(
        f"{timestamp} | "
        f"roll: {data['stabilizer.roll']:.2f}, "
        f"pitch: {data['stabilizer.pitch']:.2f}, "
        f"thrust: {data['stabilizer.thrust']:.2f}"
    )


def main():
    cflib.crtp.init_drivers()

    with SyncCrazyflie(URI, cf=Crazyflie(rw_cache="./cache")) as scf:
        cf = scf.cf

        # ---- Logging Setup ----
        log_config = LogConfig(name="Stab", period_in_ms=LOG_RATE_MS)
        log_config.add_variable("stabilizer.roll", "float")
        log_config.add_variable("stabilizer.pitch", "float")
        log_config.add_variable("stabilizer.thrust", "float")

        cf.log.add_config(log_config)

        if not log_config.valid:
            print("Log configuration invalid")
            sys.exit(1)

        log_config.data_received_cb.add_callback(log_callback)

        log_config.start()

    #     # Stop logging before exit
    #     log_config.stop()

    # print("Disconnected cleanly.")


if __name__ == "__main__":
    main()
