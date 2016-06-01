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

weston &
wpid=$!

# BUG: The first time these tests are run after reboot/restart, it crashes. This is a workaraound that should be removed
./libpelagicontain/unit-test/pelagicontainLibTest 

./libpelagicontain/unit-test/pelagicontainLibTest \
    --gtest_filter=-PelagicontainApp.FileGatewayReadOnly:PelagicontainApp.TestPulseAudioEnabled \
    --gtest_output=xml
retval=$?

if ! kill $ppid > /dev/null 2>&1 ; then
    echo "Failed to kill pulseaudio at pid $ppid"
fi

if ! kill $wpid > /dev/null 2>&1 ; then
    echo "Failed to kill weston at pid $wpid"
fi

exit $retval
