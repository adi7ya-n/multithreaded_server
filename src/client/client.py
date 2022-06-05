from pydoc import cli
from client_lib import *

if __name__ == "__main__":
    client = Client("aditya")
    # expect a username request from server
    # TODO : Will be replaced by a login prompt in the future.
    client.processServerMsg(msgSizes[MsgType.USERNAME_REQUEST.value])
    while(1):
        pass
