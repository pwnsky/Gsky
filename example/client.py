#! /usr/bin/python
import socket
import _thread
import json
from time import *
import time
#from Crypto.Util.number import bytes_to_long
from pwn import *

DEBUG = 1
uid = ''
tid = ''
sk = socket.socket(socket.AF_INET,socket.SOCK_STREAM)

if(DEBUG == 1):
    sk.connect(('127.0.0.1', 8080))
#else:
#    sk.connect(('pwnsky.com', 4096))
def pp_send(d):
    p =  b'PSP\x00'
    p += b'\x00\x00\x00\x00'
    p += (len(d) + 0).to_bytes(4, byteorder='big', signed=False)
    #p += d
    sk.send(p)
    sk.send(d)

def pp_recv():
    s = sk.recv(12);
    length = int.from_bytes(s[8:12], byteorder='big', signed=False)
    print('router: ' + str(s[4:8]) + ' length: ' + str(length))
    body = b'' 
    while True:
        if(len(body) >= length):
            break
        body += sk.recv(1)
    return body
print('send...')
pp_send(b'abc')
print("recv: ")
print(pp_recv())
