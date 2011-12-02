#!/bin/sh

if [ -z  $1 ]
then
    echo "start Client"
    udhcpc -i wlan0
else
    echo "start Daemon"
    echo 1 > /proc/sys/net/ipv4/ip_forward
    ifconfig wlan0 192.168.0.1
    udhcpd /etc/udhcpd.conf
    iptables -t nat -A POSTROUTING -o eth0 -j MASQUERADE
fi
