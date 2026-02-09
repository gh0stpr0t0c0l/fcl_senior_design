import cflib.crtp
from cflib.crazyflie import Crazyflie
from cflib.crazyflie.syncCrazyflie import SyncCrazyflie

URI = "udp://192.168.4.1:2390"

cflib.crtp.init_drivers()

with SyncCrazyflie(URI, cf=Crazyflie()) as scf:
    print("Connected")
