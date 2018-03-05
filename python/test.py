#!/usr/bin/python

import os
import socket
import pysexp

sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.connect(( 'localhost', 2709))

sock.send("S\r\n")

reader = sock.makefile('rb', 65532)

#test = "(n0 (cpuinfo (user 1) (system 1))(avenrun (1)))"
test=""

line = reader.readline()
print "read: %s"%(line)

while not line == "\n":
	test = test + line
	line = reader.readline()
	print "read: %s"%(line)

print "got: %s"%(test)

#test="((n-1 ((cpuinfo (mask 0x1) (node 0x104000a) (user 818850 890795 1408606 342835) (nice 1037 1285 1818 498) (system 645674 170505 306031 73840))(avenrun (mask 0x2) (node 0x104000a) (avenrun 0 82 29 1))(paging (mask 0x4) (node 0x104000a) (pgpgin 10957834) (pgpgout 87122990) (pswpin 63225) (pswpout 92396))(switch (mask 0x8) (node 0x104000a) (switch 500974983))(time (mask 0x10) (node 0x104000a) (timestamp 0x4aef9de7) (jiffies 51869081))(netinfo (mask 0x20) (node 0x104000a) (name lo eth0 eth1 myri0) (rxbytes 1451959993 406853376 1580448122 0) (rxpackets 3250956 2069344 149849254 0) (rxerrs 0 0 0 0) (rxdrop 0 0 0 0) (rxfifo 0 0 0 0) (rxframe 0 0 0 0) (rxcompressed 0 0 0 0) (rxmulticast 0 0 0 0) (txbytes 1451959993 784732193 1533941160 0) (txpackets 3250956 2378754 149890084 0) (txerrs 0 0 0 0) (txdrop 0 0 0 0) (txfifo 0 0 0 0) (txcolls 0 0 0 0) (txcarrier 0 0 0 0) (txcompressed 0 0 0 0))(meminfo (mask 0x40) (node 0x104000a) (total 258150) (free 12378) (buffer 2560) (shared 0) (totalhigh 32624) (freehigh 8018))(swapinfo (mask 0x80) (node 0x104000a) (total 246997) (used 9213) (pagecache 21721))(fsinfo (mask 0x100) (node 0x104000a) (devname rootfs /dev/root /proc usbdevfs none none /dev/hda1 /dev/hda6 /dev/hda5 none) (mountpoint / / proc usb pts shm scratch usr var bpfs) (fstype rootfs ext3 proc usbdevfs devpts tmpfs ext3 ext3 ext3 bpfs) (size 0 482217 0 0 0 129075 101107 16913589 1925167 0) (available 0 190290 0 0 0 129075 86689 312087 1575932 0)))))"

#t1 = pysexp.parse( "%s))"%(test) )
t1 = pysexp.parse( test )

print "==========================================================="
print t1
print "==========================================================="

for node in t1.keys():
	print "node: " + node 
	print t1[node]
	print "-----------------------------------------------"


print "============================================="

#print t1['cpuinfo']
print "============================================="

t2 = {}

t2['mask'] = 0x20
t2['cpuinfo'] = [ 1, 2, 3, 4]

print t2

while(1):
	t1 = pysexp.parse( test )
