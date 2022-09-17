from pydoc import cli
from lib import *

if __name__ == "__main__":
    client = Client("aditya")
    while (not client.gameOver_):
        client.processServerMsg()
