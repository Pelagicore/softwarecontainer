#!/usr/bin/env bash
echo "Setting up environment in $1..."
CONFIGDIR=$1/config/
ROOTFSDIR=$1/rootfs/
mkdir -p $CONFIGDIR
mkdir -p $ROOTFSDIR

cat > $CONFIGDIR/pelagicontain.conf << EOF
{
    "dbus-proxy-config-system":  [],
    "dbus-proxy-config-session": [],
    "iptables-rules": [
	"iptables -I FORWARD --src \$SRC_IP  -j ACCEPT",
	"iptables -I FORWARD --dest \$SRC_IP -j ACCEPT"
    ],
    "lxc-config-template": "/etc/pelagicontain",
    "bandwidth-limit": "500kbps"
}
EOF

