#!/bin/sh

# Copyright (C) 2016-2017 Pelagicore AB
#
# Permission to use, copy, modify, and/or distribute this software for
# any purpose with or without fee is hereby granted, provided that the
# above copyright notice and this permission notice appear in all copies.
#
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
# WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR
# BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES
# OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
# WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
# ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
# SOFTWARE.
#
# For further information see LICENSE

# Check for the bridge, and optionally create it.
#
# Usage: $0 <bridge-device>
#        $0 <bridge-device> <bridge-ip> <bridge-netmask> <bridge-netmask-bits> <bridge-netaddr>
#
# The first form won't create a bridge if not existing, the second form will create a bridge
#

if [ -z "$1" ]; then
    echo "No BRIDGE_DEVICE supplied, exiting"
    exit 1
fi

BRIDGE_DEVICE="$1"
BRIDGE_AVAILABLE=$(brctl show | grep "$BRIDGE_DEVICE")

if [ -z "$BRIDGE_AVAILABLE" ]; then
    NOARGS="$#"
    if [ "$NOARGS" -eq 1 ]; then
        echo "Bridge not available, and we're not creating it, exiting"
        exit 1
    elif [ "$NOARGS" -ne 5 ]; then
        echo "Need to supply ip, netmask, netmask bits and netaddr for bridge, in that order."
        exit 1
    fi

    BRIDGE_IP="$2"
    BRIDGE_NETMASK="$3"
    BRIDGE_NETMASK_BITS="$4"
    BRIDGE_NETADDR="$5"

    echo "$BRIDGE_DEVICE not found, attempting to add it"
    set -e

    brctl addbr "$BRIDGE_DEVICE"
    brctl setfd "$BRIDGE_DEVICE" 0
    ifconfig "$BRIDGE_DEVICE" "$BRIDGE_IP" netmask "$BRIDGE_NETMASK" promisc up
    iptables -t nat -A POSTROUTING -s "${BRIDGE_NETADDR}/${BRIDGE_NETMASK_BITS}" ! -d "${BRIDGE_NETADDR}/${BRIDGE_NETMASK_BITS}" -j MASQUERADE
    echo 1 > /proc/sys/net/ipv4/ip_forward

    exit 0
fi
