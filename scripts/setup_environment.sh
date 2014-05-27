#!/usr/bin/env bash

# README:
# -c: clean, this means the directory specified with -d should be removed
#     before creating it again
# -d: deployment directory, where to place container files
# -x: Path to controller binary
# -a: Application ID (as identified in PAM)
#
# Additional arguments: Files to put in <deployment dir>/<appId>/bin/
#
# Example: ./setup_environment.sh -d /tmp/container -d /tmp/container/
#                                 -a com.pelagicore.myApp -c yes
#                                 -- /etc/password

BRCTL_CMD="/sbin/brctl"
BRIDGE="container-br0"


# Parse command line arguments
clean= deploydir=

while getopts c:d:a:x: opt; do
    case $opt in
    c)
        clean=$OPTARG
        ;;
    d)
        deploydir=$OPTARG
        ;;
    x)
        controllerbin=$OPTARG
        ;;
    a)
        appId=$OPTARG
        ;;
    esac
done
shift $((OPTIND - 1))

# Set deployment directory
if [[ -z "$deploydir" ]]; then
    echo "Must specify -d (deployment directory)"
    exit
fi

# Check for controller
if [ -z "$controllerbin" ]; then
    echo "Must specify -x (controller binary)"
    exit
fi

if  [ ! -e $controllerbin ]; then
    echo "$controllerbin does not exist"
    exit
fi

# Check for appId
if [ -z "$appId" ]; then
    echo "Must specify -a (application id)"
    exit
fi

# Check if deploy directory should be cleaned
if [ "$clean" == "yes" ]; then
    if [ -d "$deploydir/$appId" ]; then
        echo "Unmounting $deploydir/late_mounts"
        sudo umount $deploydir/late_mounts
        echo "Removing $deploydir"
        rm -rf $deploydir
    else
        echo "This does not appear to be a deployment directory ($appId/
              missing)"
    fi
fi

# Set up system
echo "Checking system prerequisites..."
if [[ -n $($BRCTL_CMD show | grep $BRIDGE) ]]; then
    echo "Found $BRIDGE"
else
    echo "$BRIDGE was NOT FOUND, attempting to add..."
    sudo brctl addbr $BRIDGE
    sudo brctl setfd $BRIDGE 0
    sudo ifconfig $BRIDGE 10.0.3.1 netmask 255.255.255.0 promisc up
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
mountdir="$deploydir$mountname"
grepresult=(`mount | grep "$mountdir"`)

if [ "${grepresult[0]}" = "$mountdir" ] && [ "${grepresult[0]}" = "${grepresult[2]}" ]; then
    echo "Found circular bind-mount (this is good)"
else
    echo "Circular bind mount on $mountdir NOT FOUND, attempting to add..."
    sudo mount --bind $mountdir $mountdir
    sudo mount --make-unbindable $mountdir
    sudo mount --make-shared $mountdir
fi

# Create directory structure
ROOTFSDIR=$deploydir/
echo "Setting up environment in $ROOTFSDIR"
set -x
mkdir -p $ROOTFSDIR
mkdir -p $ROOTFSDIR/bin
mkdir -p $ROOTFSDIR/$appId
mkdir -p $ROOTFSDIR/$appId/bin
mkdir -p $ROOTFSDIR/$appId/home
mkdir -p $ROOTFSDIR/$appId/shared
cp $controllerbin $ROOTFSDIR/bin
set +x

# Deploy files in rootfs/
for file in $@; do
    cp -a $file $ROOTFSDIR/$appId/bin
done
