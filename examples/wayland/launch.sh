#!/bin/sh

# Copyright (C) 2016 Pelagicore AB
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


# This is an example of a misbehaving application being launched in a
# container, the example will launch simple.c in a container, and observe that
# it is killed inside the container by the OOM killer.

help() {
    echo "$0 [-b [system|session]]"
    echo ""
    echo "  -b    what type of dbus to use, session or system"
    exit
}


BUS="system"
while getopts ":b:h" opt; do
    case $opt in
    b)
        BUS="$OPTARG"
        ;;
    h)
        help
        ;;
    *)
        echo "Unknown option $OPTARG"
        help
    esac
done

SIMPLEWESTON=$(which weston-simple-egl)
if [ "$?" != "0" ]; then
    echo "weston-simple-egl not found"
    exit 1
fi

FLOWERWESTON=$(which weston-flower)
if [ "$?" != "0" ]; then
    echo "weston-flower not found"
    exit 1
fi

# Create a new session D-Bus bus
eval `dbus-launch --sh-syntax`

# Print the environment
env

# Launch weston
export XDG_RUNTIME_DIR=/tmp/wayland/
mkdir -p $XDG_RUNTIME_DIR
chmod 0700 $XDG_RUNTIME_DIR
weston &
WESTONPID="$!"

# Launch a new agent
softwarecontainer-agent &
AGENTPID="$!"

# Let the agent start up
sleep 2

# Destination
PCNAME="com.pelagicore.SoftwareContainerAgent"
# Object path
PCOBJPATH="/com/pelagicore/SoftwareContainerAgent"
# Prefix for dbus methods
AGENTPREFIX="com.pelagicore.SoftwareContainerAgent"
export PCCMD="dbus-send --${BUS} --print-reply --dest=$PCNAME $PCOBJPATH"

# Introspect the agent
$PCCMD org.freedesktop.DBus.Introspectable.Introspect

# Ping the agent
$PCCMD $AGENTPREFIX.Ping

# Create a new container
$PCCMD $AGENTPREFIX.CreateContainer string:prefix string:'[{"writeOften": "0"}]'

# A few thing that we use for more or less every call below
CONTAINERID="uint32:0"
ROOTID="uint32:0"
OUTFILE="/tmp/stdout"

# Set gateway config allowing wayland access
WAYLANDKEY="wayland"
WAYLANDVALUE="[{\"enabled\": true}]"

# Enable wayland gateway
$PCCMD $AGENTPREFIX.SetGatewayConfigs \
    $CONTAINERID \
    dict:string:string:"$WAYLANDKEY","$WAYLANDVALUE"

# Run the simple egl example from weston
$PCCMD $AGENTPREFIX.LaunchCommand \
    $CONTAINERID \
    $ROOTID \
    string:"$SIMPLEWESTON" \
    string:/gateways/app \
    string:${OUTFILE}-simple \
    dict:string:string:""

# Run weston-flower
$PCCMD $AGENTPREFIX.LaunchCommand \
    $CONTAINERID \
    $ROOTID \
    string:"$FLOWERWESTON" \
    string:/gateways/app \
    string:${OUTFILE}-flower \
    dict:string:string:""

# Let the examples run for a while
sleep 30

# Shutdown the container
$PCCMD $AGENTPREFIX.ShutDownContainer $CONTAINERID

# Clean up
kill $WESTONPID
kill $AGENTPID
