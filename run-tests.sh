#!/bin/bash

eval $(dbus-launch --sh-syntax)
echo "D-Bus per-session daemon address is: $DBUS_SESSION_BUS_ADDRESS"

# TODO: pulseaudio needs to be setup properly for PelagicontainApp.TestPulseAudioEnabled to work
pulseaudio --system &
pid=$!

# BUG: The first time these tests are run after reboot/restart, it crashes. This is a workaraound that should be removed
./libpelagicontain/unit-test/pelagicontainLibTest 

./libpelagicontain/unit-test/pelagicontainLibTest \
    --gtest_filter=-PelagicontainApp.FileGatewayReadOnly:PelagicontainApp.TestPulseAudioEnabled \
    --gtest_output=xml
retval=$?

if ! kill $pid > /dev/null 2>&1 ; then
    echo "Failed to kill pulseaudio at pid $pid"
fi

exit $retval
