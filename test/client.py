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
key = []

def pp_connect():
    p =  b'\x50\x50\x10\x00'
    p +=  (0).to_bytes(4, byteorder='big', signed=False)
    p += b'\x00' * 8
    sk.send(p)
    print(p)
    key = pp_recv()
    print('key: ')
    print(key)


def pp_send(d):
    p =  b'PP\x10\x00'
    p += (len(d) + 0).to_bytes(4, byteorder='big', signed=False)
    p += b'\x00' * 8
    p += d
    print(p)
    sk.send(p)
    sk.send(d)

def pp_recv():
    s = sk.recv(16);
    print('recv:')
    print(s)
    '''
    status = s[2:3]
    type_  = s[3:4]
    length = int.from_bytes(s[4:8], byteorder='big', signed=False)
    print('router: ' + str(s[8:16]) + ' length: ' + str(length) + ' status : 0x%02x' % ord(status))

    body = b'' 
    while True:
        if(len(body) >= length):
            break
        body += sk.recv(1)
    return body
    '''
    return 'a'

print('pp connecting')
pp_connect()

