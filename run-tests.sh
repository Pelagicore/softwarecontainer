#!/bin/bash

if [ $UID != 0 ]; then
    echo "This script must be run as root"
    exit 1
fi

eval $(dbus-launch --sh-syntax)
echo "D-Bus per-session daemon address is: $DBUS_SESSION_BUS_ADDRESS"

# TODO: pulseaudio needs to be setup properly for PelagicontainApp.TestPulseAudioEnabled to work
pulseaudio --system &
ppid=$!

export XDG_RUNTIME_DIR=/run/user/$UID/wayland/
mkdir -p $XDG_RUNTIME_DIR
chmod 0700 $XDG_RUNTIME_DIR
echo "XDG_RUNTIME_DIR is $XDG_RUNTIME_DIR"
weston --backend=headless-backend.so &
wpid=$!

# BUG: The first time these tests are run after reboot/restart, it crashes. This is a workaraound that should be removed
./libpelagicontain/unit-test/pelagicontainLibTest 

./libpelagicontain/unit-test/pelagicontainLibTest \
    --gtest_filter=-PelagicontainApp.FileGatewayReadOnly:PelagicontainApp.TestPulseAudioEnabled \
    --gtest_output=xml
retval=$?

if ! kill $wpid > /dev/null 2>&1 ; then
    echo "Failed to kill weston at pid $wpid"
fi

if ! kill $ppid > /dev/null 2>&1 ; then
    echo "Failed to kill pulseaudio at pid $ppid"
fi

if ! kill $DBUS_SESSION_BUS_PID > /dev/null 2>&1 ; then
    echo "Failed to kill D-Bus session bus at pid $DBUS_SESSION_BUS_PID"
fi

exit $retval
