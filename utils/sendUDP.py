import socket


_s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
_s.sendto((2000).to_bytes(2, byteorder='little'), ('192.168.10.56', 8080))

print('code OK')
