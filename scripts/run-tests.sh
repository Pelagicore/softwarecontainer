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

#
# Don't run this from the scripts/ directory. It is copied into the build dir
# by CMake, and it is there it should be run.
#

UID=$(id -u)
if [ $UID != 0 ]; then
    echo "This script must be run as root"
    exit 1
fi

echo "### Launching dbus session ###"
eval $(dbus-launch --sh-syntax)
echo "D-Bus per-session daemon address is: $DBUS_SESSION_BUS_ADDRESS"

echo "### Launching pulseaudio ###"
PULSE_SERVER=/tmp/pulse.sock
pulseaudio --daemonize --exit-idle-time=-1
ppid=$!
pactl load-module module-native-protocol-unix auth-anonymous=1 socket=$PULSE_SERVER
export PULSE_SERVER=$PULSE_SERVER

echo "### Starting dlt-daemon ###"
dlt-daemon &
dpid=$!

echo "### Starting dlt-receive ###"
export LD_LIBRARY_PATH=$(dirname $(dirname $(which dlt-receive)))/lib
dlt-receive -a localhost 1> dlt.log &
rpid=$!

echo "### Starting Weston ###"
export XDG_RUNTIME_DIR=/run/user/$UID/wayland/
mkdir -p $XDG_RUNTIME_DIR
chmod 0700 $XDG_RUNTIME_DIR
echo "XDG_RUNTIME_DIR is $XDG_RUNTIME_DIR"
weston --backend=headless-backend.so &
wpid=$!

echo "### Running tests ###"
FILTER_ARG=""
if [ -n "$1" ]; then
    FILTER_ARG="--gtest_filter=$1"
fi

./agent/unit-test/softwarecontaineragenttest \
    $FILTER_ARG \
    --gtest_output=xml:softwarecontaineragent_unittest_result.xml
retval=$?

./common/unit-test/softwarecontainercommontest \
    $FILTER_ARG \
    --gtest_output=xml:softwarecontainercommon_unittest_result.xml
retval=$(($retval+$?))

./libsoftwarecontainer/unit-test/softwarecontainerlibtest \
    $FILTER_ARG \
    --gtest_output=xml:softwarecontainerlib_unittest_result.xml
retval=$(($retval+$?))

if ! kill $wpid > /dev/null 2>&1 ; then
    echo "Failed to kill weston at pid $wpid"
fi

pactl exit
if kill -0 $ppid > /dev/null 2>&1 ; then
    if ! kill $ppid > /dev/null 2>&1 ; then
        echo "Failed to kill pulseaudio at pid $ppid"
    fi
fi

if ! kill $rpid > /dev/null 2>&1 ; then
    echo "Failed to kill dlt-receiver"
fi

if ! kill $dpid > /dev/null 2>&1 ; then
    echo "Failed to kill dlt-daemon"
fi

if ! kill $DBUS_SESSION_BUS_PID > /dev/null 2>&1 ; then
    echo "Failed to kill D-Bus session bus at pid $DBUS_SESSION_BUS_PID"
fi

exit $retval
