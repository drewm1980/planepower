#!/usr/bin/env python
from socket import socket, AF_PACKET, SOCK_RAW
from time import sleep

# Note, raw sockets require root permissions!
s = socket(AF_PACKET, SOCK_RAW)

# sockets the address is a tuple (ifname, proto [,pkttype [,hatype]])
s.bind(("eth0", 0))

#bbone_mac = "90:59:AF:57:3E:70"
bbone_mac = "\x90\x59\xAF\x57\x3E\x70"

#our_mac = "00:26:b9:eb:e6:a3"
our_mac = "\x00\x26\xb9\xeb\xe6\xa3"

def send(payload):
    print "Entering send"
    # We're putting together an ethernet frame here, 
    # but you could have anything you want instead
    # Have a look at the 'struct' module for more 
    # flexible packing/unpacking of binary data
    # and 'binascii' for 32 bit CRC
    #src_addr = "\x01\x02\x03\x04\x05\x06"
    #dst_addr = "\x01\x02\x03\x04\x05\x06"
    src_addr = our_mac
    dst_addr = bbone_mac
    checksum = "\x1a\x2b\x3c\x4d" # obviously garbage values
    ethertype = "\x08\x01"
    # Note, if the ethertype octet is below 0x05DC (1500), it is interpreted as the size of the message. if above, it is an ethertype.  0x0800 is ipv4.  Not sure what x0801 is...

    s.send(dst_addr+src_addr+ethertype+payload+checksum)
    print "Exiting send"

def receive(len):
    print "Entering receive"
    message = s.recv(4096)
    print "Exiting receive"
    print s
    print s.decode('hex')

import threading
sender=threading.Thread(target=send,args=(("foo",)))
receiver=threading.Thread(target=receive,args=())
sender.daemon=True
receiver.daemon=True

receiver.start()
sender.start()

print "Main thread sleeping"
sleep(1.0)
print "Main thread exiting"


