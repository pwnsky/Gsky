from pwn import *

context.log_level = 'debug'
io = remote('127.0.0.1', 8080)


io.interactive()
