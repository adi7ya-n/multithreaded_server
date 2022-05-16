import socket
import struct
import time

HOST = "127.0.0.1"
PORT = 9000

def processServerMsg(rawData: bytes):
    [msgType, data] = struct.unpack("<BB", rawData)
    print("MsgType: ", msgType, " data: ", data)

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.connect((HOST, PORT))
    print("Connected successfully to the server")
    data = s.recv(2)
    processServerMsg(data)
    name = "aditya"
    s.send(name.encode())