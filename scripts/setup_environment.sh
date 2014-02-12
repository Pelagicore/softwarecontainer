#!/usr/bin/env bash
echo "Setting up environment in $1..."
CONFIGDIR=$1/config/
ROOTFSDIR=$1/rootfs/
mkdir -p $CONFIGDIR
mkdir -p $ROOTFSDIR

cat > $CONFIGDIR/pelagicontain.conf << EOF
{
    "lxc-config-template": "/etc/pelagicontain",
}
EOF
