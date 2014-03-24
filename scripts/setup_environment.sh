#!/usr/bin/env bash

# README:
# -c: clean, this means the directory specified with -d should be removed
#     before creating it again
# -d: deployment directory, where to place container files
#
# Additional arguments: Files to put in directory specified with -d
#
# Example: ./setup_environment.sh -d /tmp/container -c yes -- /etc/passwod

BRCTL_CMD="/sbin/brctl"
BRIDGE="container-br0"


# Parse command line arguments
clean= deploydir=

while getopts c:d: opt; do
    case $opt in
    c)
        clean=$OPTARG
        ;;
    d)
        deploydir=$OPTARG
        ;;
    esac
done
shift $((OPTIND - 1))

# Set deployment directory
if [[ -z "$deploydir" ]]; then
    echo "Must specify -d (deployment directory)"
    exit
fi

# Check if deploy directory should be cleaned
if [ "$clean" == "yes" ]; then
    if [ -d "$deploydir/rootfs" ]; then
        echo "Removing $deploydir"
        rm -rf $deploydir
    else
        echo "This does not appear to be a deployment directory (rootfs/
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
fi

# Create directory structure
ROOTFSDIR=$deploydir/rootfs/
echo "Setting up environment in $ROOTFSDIR"
mkdir -p $ROOTFSDIR

# Deploy files in rootfs/
for file in $@; do
    cp -a $file $ROOTFSDIR
done
