import socket
import struct
import time
from enum import Enum

HOST = "127.0.0.1"
PORT = 9000

PACKET_FORMAT = "<BB"


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
    START_SERVER = 7
    SHUTDOWN_SERVER = 8
    DRAW_MATCH = 11
    O_WINS = 12
    X_WINS = 13


def createPacket(type, data):
    return struct.pack(PACKET_FORMAT, type, data)


def decodePacket(rawData: bytes):
    print("Received raw data: ", rawData)
    return struct.unpack(PACKET_FORMAT, rawData)


class Client:
    def __init__(self, userName: str) -> None:
        self.socket_ = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.socket_.connect((HOST, PORT))
        self.userName_ = userName
        self.board_ = [['-', '-', '-'], ['-', '-', '-'], ['-', '-', '-']]
        self.playerIdentifier_ = 'X'
        self.opponentIdentifier_ = 'O'
        self.gameOver_ = False

    def sendUsername(self):
        msg = self.userName_ + '\n'
        self.socket_.send(msg.encode('utf-8'))
        print("Sent username to server")

    def displayBoard(self):
        for row in self.board_:
            for element in row:
                print(element, end="  ")
            print()

    def getPosition(self, move):
        [rowNum, colNum] = (0, 0)
        if (move % 3 != 0):
            rowNum = int(move / 3)
        else:
            rowNum = int(move / 3) - 1
        colNum = move - 3 * rowNum - 1
        print("R,C: ", rowNum, colNum)
        return (rowNum, colNum)

    def isLocationOccupied(self, move):
        (rowNum, colNum) = self.getPosition(move)
        if(self.board_[rowNum][colNum] != '-'):
            return True
        else:
            return False

    def updateBoard(self, move, id):
        (rowNum, colNum) = self.getPosition(move)
        self.board_[rowNum][colNum] = id
        print("The board is now: ")
        self.displayBoard()

    def isInteger(self, move):
        return move.isdigit()

    def isValidMove(self, move):
        if(not self.isInteger(move)):
            print("Entered value is not a number.")
            return False
        elif(not (1 <= int(move) <= 9)):
            print("Entered value is not in the range 1 to 9.")
            return False
        elif(self.isLocationOccupied(int(move))):
            print("Location is already occupied.")
            return False
        else:
            return True

    def readMove(self) -> int:
        move = input("Enter a number between 1 and 9: ")
        while(not self.isValidMove(move)):
            move = input("Enter a number between 1 and 9: ")
        return int(move) + 100

    def sendResponse(self):
        move = self.readMove()
        self.socket_.send(createPacket(
            PacketType.DATA_PACKET.value, 9))
        # x = int(input("Enter test number: "))
        self.socket_.send(createPacket(PacketType.DATA_PACKET.value, move))
        self.updateBoard(move, self.playerIdentifier_)

    def handleIncomingMsg(self, rawData: bytes):
        [msgType, data] = decodePacket(rawData)
        print("RECVD MSG FROM SERVER: MsgType: ", msgType, " data: ", data)
        if(msgType == PacketType.CONN_PACKET.value):
            if(data == MsgType.USERNAME_REQUEST.value):
                self.sendUsername()
            elif(data == MsgType.PLAYER1_INDICATION.value):
                print("You are X!")
                self.opponentIdentifier_ = 'O'
                self.sendResponse()
            elif(data == MsgType.PLAYER2_INDICATION.value):
                self.playerIdentifier_ = 'O'
                self.opponentIdentifier_ = 'X'
                print("You are O!")

        elif(msgType == PacketType.DATA_PACKET.value):
            if(data == MsgType.X_WINS.value):
                if(self.playerIdentifier_ == 'X'):
                    print("You won!!")
                else:
                    print("You lost :(")
                self.gameOver_ = True
                return
            elif(data == MsgType.DRAW_MATCH.value):
                print("Draw match!!")
                self.gameOver_ = True
            elif(data == MsgType.O_WINS.value):
                if(self.playerIdentifier_ == 'O'):
                    print("You won!!")
                else:
                    print("You lost :(")
                self.gameOver_ = True
                return
            print("Received move from player!")
            self.updateBoard(data-100, self.opponentIdentifier_)
            self.sendResponse()

    def processServerMsg(self):
        self.handleIncomingMsg(self.socket_.recv(2))


class Admin:
    def __init__(self) -> None:
        self.socket_ = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.socket_.connect((HOST, PORT))

    def startServer(self):
        self.socket_.send(createPacket(
            PacketType.ADMIN_PACKET, MsgType.START_SERVER))

    def shutDownServer(self):
        self.socket_.send(createPacket(
            PacketType.ADMIN_PACKET, MsgType.SHUTDOWN_SERVER))
