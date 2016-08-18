#!/bin/sh

#
# Don't run this from the scripts/ directory. It is copied into the build dir
# by CMake, and it is there it should be run.
#

UID=$(id -u)
if [ $UID != 0 ]; then
    echo "This script must be run as root"
    exit 1
fi

eval $(dbus-launch --sh-syntax)
echo "D-Bus per-session daemon address is: $DBUS_SESSION_BUS_ADDRESS"

PULSE_SERVER=/tmp/pulse.sock
pulseaudio --daemonize
ppid=$!
pactl load-module module-native-protocol-unix auth-anonymous=1 socket=$PULSE_SERVER
export PULSE_SERVER=$PULSE_SERVER

# BUG: ivi-logging and dlt does not work together in softwarecontainer for some reason
#      Everything in the softwarecontainerLibTest hangs and stops dead if DLT backend 
#      is enabled.
# # Setup environment for tests
# echo "### Starting dlt-daemon ###"
# dlt-daemon &
# dpid=$!
# 
# echo "### Starting dlt-receive ###"
# export LD_LIBRARY_PATH=$(dirname $(dirname $(which dlt-receive)))/lib
# dlt-receive -a localhost 1> $logName &
# rpid=$!

export XDG_RUNTIME_DIR=/run/user/$UID/wayland/
mkdir -p $XDG_RUNTIME_DIR
chmod 0700 $XDG_RUNTIME_DIR
echo "XDG_RUNTIME_DIR is $XDG_RUNTIME_DIR"
weston --backend=headless-backend.so &
wpid=$!

GTEST_FILTER="-*FileGatewayReadOnly"
GTEST_OPTS="--gtest_output=xml"
if [ -n "$1" ]; then
    GTEST_FILTER="$1"
fi

./libsoftwarecontainer/unit-test/softwarecontainerLibTest \
    --gtest_filter=$GTEST_FILTER \
    $GTEST_OPTS
retval=$?

if ! kill $wpid > /dev/null 2>&1 ; then
    echo "Failed to kill weston at pid $wpid"
fi

pactl exit
if kill -0 $ppid > /dev/null 2>&1 ; then
    if ! kill $ppid > /dev/null 2>&1 ; then
        echo "Failed to kill pulseaudio at pid $ppid"
    fi
fi

# BUG: ivi-logging and dlt does not work together in softwarecontainer for some reason
#      Everything in the softwarecontainerLibTest hangs and stops dead if DLT backend 
#      is enabled.
# # Shutdown the environment used by the tests
# if ! kill $rpid > /dev/null 2>&1 ; then
#     echo "Failed to kill dlt-receiver"
# fi
# 
# if ! kill $dpid > /dev/null 2>&1 ; then
#     echo "Failed to kill dlt-daemon"
# fi
# 
if ! kill $DBUS_SESSION_BUS_PID > /dev/null 2>&1 ; then
    echo "Failed to kill D-Bus session bus at pid $DBUS_SESSION_BUS_PID"
fi

exit $retval
