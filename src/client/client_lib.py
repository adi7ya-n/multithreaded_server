import socket
import struct
import time
from enum import Enum

HOST = "127.0.0.1"
PORT = 9000


class PacketType(Enum):
    CONN_PACKET = 0xAA
    DATA_PACKET = 0xBB
    ADMIN_PACKET = 0xCC


class MsgType(Enum):
    USERNAME_REQUEST = 0
    NUM_OF_GAMES = 1
    REBOOT_SERVER = 2
    DISPLAY_ONGOING_GAMES = 3
    GET_GAME_INFO = 4
    PLAYER1_INDICATION = 5
    PLAYER2_INDICATION = 6


msgSizes = {MsgType.USERNAME_REQUEST.value: 2}


class Client:
    def __init__(self, userName: str) -> None:
        self._socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self._socket.connect((HOST, PORT))
        self._userName = userName

    def sendUsername(self):
        msg = self._userName + '\n'
        self._socket.send(msg.encode('utf-8'))
        print("Sent username to server")

    def sendMove(self):
        pass

    def handleIncomingMsg(self, rawData: bytes):
        [msgType, data] = struct.unpack("<BB", rawData)
        print("RECVD MSG FROM SERVER: MsgType: ", msgType, " data: ", data)
        if(msgType == PacketType.CONN_PACKET.value):
            if(data == MsgType.USERNAME_REQUEST.value):
                self.sendUsername()

    def processServerMsg(self, size: int):
        self.handleIncomingMsg(self._socket.recv(size))
