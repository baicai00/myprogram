import socket

s = socket.socket()
host = "127.0.0.1"
port = 8887
s.connect((host, port))
s.send('hello world')
s.close()