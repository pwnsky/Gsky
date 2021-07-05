#! /usr/bin/python
import socket
import json
from time import *
import time


PEXorTable = [
        0xbe, 0xd1, 0x90, 0x88, 0x57, 0x00, 0xe9, 0x53, 0x10, 0xbd, 0x2a, 0x34, 0x51, 0x84, 0x07, 0xc4, 
        0x33, 0xc5, 0x3b, 0x53, 0x5f, 0xa8, 0x5d, 0x4b, 0x6d, 0x22, 0x63, 0x5d, 0x3c, 0xbd, 0x47, 0x6d, 
        0x22, 0x3f, 0x38, 0x4b, 0x7a, 0x4c, 0xb8, 0xcc, 0xb8, 0x37, 0x78, 0x17, 0x73, 0x23, 0x27, 0x71, 
        0xb1, 0xc7, 0xa6, 0xd1, 0xa0, 0x48, 0x21, 0xc4, 0x1b, 0x0a, 0xad, 0xc9, 0xa5, 0xe6, 0x14, 0x18, 
        0xfc, 0x7b, 0x53, 0x59, 0x8b, 0x0d, 0x07, 0xcd, 0x07, 0xcc, 0xbc, 0xa5, 0xe0, 0x28, 0x0e, 0xf9, 
        0x31, 0xc8, 0xed, 0x78, 0xf4, 0x75, 0x60, 0x65, 0x52, 0xb4, 0xfb, 0xbf, 0xac, 0x6e, 0xea, 0x5d, 
        0xca, 0x0d, 0xb5, 0x66, 0xac, 0xba, 0x06, 0x30, 0x95, 0xf4, 0x96, 0x42, 0x7a, 0x7f, 0x58, 0x6d, 
        0x83, 0x8e, 0xf6, 0x61, 0x7c, 0x0e, 0xfd, 0x09, 0x6e, 0x42, 0x6b, 0x1e, 0xb9, 0x14, 0x22, 0xf6, 

        0x16, 0xd2, 0xd2, 0x60, 0x29, 0x23, 0x32, 0x9e, 0xb4, 0x82, 0xee, 0x58, 0x3a, 0x7d, 0x1f, 0x74, 
        0x98, 0x5d, 0x17, 0x64, 0xe4, 0x6f, 0xf5, 0xad, 0x94, 0xaa, 0x89, 0xe3, 0xbe, 0x98, 0x91, 0x38, 
        0x70, 0xec, 0x2f, 0x5e, 0x9f, 0xc9, 0xb1, 0x26, 0x3a, 0x64, 0x48, 0x13, 0xf1, 0x1a, 0xc5, 0xd5, 
        0xe5, 0x66, 0x11, 0x11, 0x3a, 0xaa, 0x79, 0x45, 0x42, 0xb4, 0x57, 0x9d, 0x3f, 0xbc, 0xa3, 0xaa, 
        0x98, 0x4e, 0x6b, 0x7a, 0x4a, 0x2f, 0x3e, 0x10, 0x7a, 0xc5, 0x33, 0x8d, 0xac, 0x0b, 0x79, 0x33, 
        0x5d, 0x09, 0xfc, 0x9d, 0x9b, 0xe5, 0x18, 0xcd, 0x1c, 0x7c, 0x8b, 0x0a, 0xa8, 0x95, 0x56, 0xcc, 
        0x4e, 0x34, 0x31, 0x33, 0xf5, 0xc1, 0xf5, 0x03, 0x0a, 0x4a, 0xb4, 0xd1, 0x90, 0xf1, 0x8f, 0x57, 
        0x20, 0x05, 0x0d, 0xa0, 0xcd, 0x82, 0xb3, 0x25, 0xd8, 0xd2, 0x20, 0xf3, 0xc5, 0x96, 0x35, 0x35, 
    ]


def PE_Encode(keys, data):
    key_arr = []
    data_arr = []
    for c in keys:
        key_arr.append(c)

    for c in data:
        data_arr.append(c)
    keys = key_arr
    data = data_arr

    for i in range(len(data)):
        n = ((keys[i & 7] + keys[(i + 1) & 7]) * keys[(i + 2) & 7] + keys[(i + 2) & 7]) & 0xff
        data[i] ^= n ^ PEXorTable[n]
        keys[i & 7] = (n * 2 + 3) & 0xff
        if((i & 0xf) == 0): //密钥重置
            PE_KeyRandom(keys, seed)

    out = b''
    for c in data:
        out += c.to_bytes(1, byteorder='little')
    return out

def PE_Decode(keys, data):
    key_arr = []
    data_arr = []
    for c in keys:
        key_arr.append(c)

    for c in data:
        data_arr.append(c)
    keys = key_arr
    data = data_arr

    for i in range(len(data)):
        t_key = keys[i % 8]
        n = ((keys[i % 8] + keys[(i + 1) % 8]) * keys[(i + 2) % 8]) & 0xff;
        data[i] ^= n ^ PEXorTable[n]
        data[i] ^= t_key
        keys[i % 8] = (n * 2 + 3) % 0x100

    out = b''
    for c in data:
        out += c.to_bytes(1, byteorder='little')
    return out

DEBUG = 0
uid = ''
tid = ''
sk = socket.socket(socket.AF_INET,socket.SOCK_STREAM)

if(DEBUG == 1):
    sk.connect(('pwnsky.com', 4096))
else:
    sk.connect(('127.0.0.1', 4096))

key = b'\x00' * 8
code = b''

def pp_connect():
    global key
    p =  b'\x50\x50\x10\x00'
    p +=  (0).to_bytes(4, byteorder='big', signed=False)
    p += b'\x00' * 8
    sk.send(p)
    print('request get key: ', end='')
    (status, data) = pp_recv()
    if(status == 0x31 and key == b'\x00' * 8): # 状态代码为0x31为服务端发送key
        key = data
    #print(b'key : ' + key)
    #print(b'code : ' + code)

def pp_send(d, route):
    p =  b'\x50\x50\x11\x00'
    p += (len(d) + 0).to_bytes(4, byteorder='big', signed=False)
    route = route.ljust(6, b'\x00')
    sk.send(p + PE_Encode(key, route + code))
    d = PE_Encode(key, d)
    writed_len = 0;
    while True:
        if(writed_len >= len(d)):
            print('发送')
            print(d)
            print('发送完毕')
            break;
        write_len = sk.send(d[writed_len:])
        if(write_len < 0):
            continue
        elif(write_len == 0):
            print('disconnect')
            break
        else:
            writed_len += write_len

    print(b'key: ' + key)
    print(b'code: ' + code)

def pp_recv():
    global code, key
    s = sk.recv(8);
    status = s[2:3]
    type_  = s[3:4]
    length = int.from_bytes(s[4:8], byteorder='big', signed=False)

    s = sk.recv(8) # 接收剩余头部
    s = PE_Decode(key, s)

    print('route: ' + str(s[0:6]) + ' length: ' + str(length) + ' status : 0x%02x' % ord(status))
    print('code: ' + str(s[6:8]))
    if(code == b''):
        code = s[6:8]
    body = b'' 
    while True:
        if(len(body) >= length):
            break
        body += sk.recv(1)
        if(len(body) == 200): # just for test
            sleep(4)

    body = PE_Decode(key, body)
    return (ord(status), body)

print('------------------- Test Connect -------------------')
print('pp connecting')
pp_connect()

print('------------------- Test Error -------------------')
pp_send(b'hello pp!, Test Error', b'\x00') # Test Error
(status, d) = pp_recv()
if(status == 0x30): # 状态代码为OK
    print(b'recv: ' + d)

print('------------------- Test Echo -------------------')
pp_send(b'Test Echo', b'\x10') # Test Echo
(status, d) = pp_recv()
if(status == 0x30): # 状态代码为OK
    print(b'recv: ' + d)
print('------------------- Test Push -------------------')
pp_send(b'Test Push', b'\x00') # Test Push
(status, d) = pp_recv()
if(status == 0x30): # 状态代码为OK
    print(b'recv: ' + d)
print('------------------- Test Multi Server Push -------------------')
pp_send(b'Test Multi Push', b'\x30') # Test Multi Push
(status, d) = pp_recv()
if(status == 0x30): # 状态代码为OK
    print(b'recv: ' + d)

(status, d) = pp_recv() # recv again
if(status == 0x30): # 状态代码为OK
    print(b'recv: ' + d)


'''
pp_send(b'' * 0x10, b'\x20')
(status, d) = pp_recv()
if(status == 0x30): # 状态代码为OK
    print(b'recv: ' + d)
(status, d) = pp_recv()
if(status == 0x30): # 状态代码为OK
    print('recved data again')
    print(b'recv: ' + d)
'''
sk.close()
