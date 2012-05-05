#!/bin/sh

killall udhcpd 2>/dev/null

PIDS=`ps | grep "udhcpc -i wlan0" | cut -d' ' -f2`
echo "PIDS: $PIDS"

for i in $PIDS; do
    kill -9  $i 2>/dev/null
done
