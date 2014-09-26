#!/bin/bash -e

# Example: ./setup_pelagicontain.sh /tmp/container

BRCTL_CMD="brctl"
BRIDGE="container-br0"

clean=yes
deploydir=$1

if [ -z "$1" ]
  then
    echo "Must specify deployment directory"
fi

if [ -d "$deploydir" ]; then
    echo "Unmounting $deploydir/late_mounts"
    umount $deploydir/late_mounts
    echo "Removing $deploydir"
    rm -rf $deploydir
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

if [ -d "$deploydir/late_mounts" ]; then
    echo "Found $deploydir/late_mounts"
else
    echo "$deploydir/late_mounts was NOT FOUND, attempting to add..."
    mkdir -p "$deploydir/late_mounts"
fi

# Check for late mounts
if [ -d "$deploydir/late_mounts" ]; then
    echo "Found $deploydir/late_mounts"
else
    echo "$deploydir/late_mounts was NOT FOUND, attempting to add..."
    mkdir -p "$deploydir/late_mounts"
fi

# Check for circular mounts. Mount if not found.
mountname="late_mounts"
mountdir="$deploydir/$mountname"

mount --bind $mountdir $mountdir
mount --make-unbindable $mountdir
mount --make-shared $mountdir

# Create directory structure
#ROOTFSDIR=$deploydir/
#echo "Setting up environment in $ROOTFSDIR"
#set -x
#set +x


