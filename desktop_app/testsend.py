# udp_test_send.py
import socket
s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
s.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
s.sendto(b"hello-from-pc\n", ("255.255.255.255", 1234))
s.close()
