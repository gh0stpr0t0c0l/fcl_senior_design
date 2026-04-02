import cflib.crtp
from cflib.crazyflie import Crazyflie
from cflib.crazyflie.syncCrazyflie import SyncCrazyflie

URI = "udp://192.168.43.42:2390"


def print_log_toc(cf):
    for group in cf.log.toc.toc:
        for var in cf.log.toc.toc[group]:
            print(f"{group}.{var}")


if __name__ == "__main__":
    cflib.crtp.init_drivers()

    with SyncCrazyflie(URI, cf=Crazyflie(rw_cache="./cache")) as scf:
        print_log_toc(scf.cf)
