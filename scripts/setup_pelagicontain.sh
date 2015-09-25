#!/bin/bash -e

BRCTL_CMD="brctl"
BRIDGE="lxcbr0"

# Set up system
echo "Checking system prerequisites..."
if [[ -n $($BRCTL_CMD show | grep $BRIDGE) ]]; then
    echo "Found $BRIDGE"
else
    echo "$BRIDGE was NOT FOUND, attempting to add..."
    $BRCTL_CMD addbr $BRIDGE
    $BRCTL_CMD setfd $BRIDGE 0
    ifconfig $BRIDGE 10.0.3.1 netmask 255.255.255.0 promisc up
    iptables -t nat -A POSTROUTING -s 10.0.3.0/24 ! -d 10.0.3.0/24 -j MASQUERADE
    echo 1 > /proc/sys/net/ipv4/ip_forward
fi
