#!/bin/bash -e

# Example: ./setup_pelagicontain.sh /tmp/container

BRCTL_CMD="brctl"
BRIDGE="lxcbr0"

clean=yes
deploydir=$1

if [ -z "$1" ]
  then
    echo "Must specify deployment directory"
fi

# Set up system
echo "Checking system prerequisites..."
if [[ -n $($BRCTL_CMD show | grep $BRIDGE) ]]; then
    echo "Found $BRIDGE"
else
    echo "$BRIDGE was NOT FOUND, attempting to add..."
    brctl addbr $BRIDGE
    brctl setfd $BRIDGE 0
    ifconfig $BRIDGE 10.0.3.1 netmask 255.255.255.0 promisc up
    iptables -t nat -A POSTROUTING -s 10.0.3.0/24 ! -d 10.0.3.0/24 -j MASQUERADE
    echo 1 > /proc/sys/net/ipv4/ip_forward
fi
