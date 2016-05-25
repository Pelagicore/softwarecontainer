#!/bin/bash

eval $(dbus-launch --sh-syntax)
echo "D-Bus per-session daemon address is: $DBUS_SESSION_BUS_ADDRESS"

./libpelagicontain/unit-test/pelagicontainLibTest \
    --gtest_filter=-PelagicontainApp.FileGatewayReadOnly:PelagicontainApp.TestPulseAudioEnabled \
    --gtest_output=xml


