#!/bin/bash

# Parse command line arguments
pelagicontain_bin=../../build/pelagicontain/src/pelagicontain
container_path=/tmp/container/
controller_bin=../../build/controller/src/controller
setup_script=../../scripts/setup_environment.sh

while getopts p:c:x:s opt; do
    case $opt in
    p)
        pelagicontain_bin=$OPTARG
        ;;
    c)
        container_path=$OPTARG
        ;;
    x)
        controller_bin=$OPTARG
        ;;
    s)
        setup_script=$OPTARG
        ;;
    esac
done
shift $((OPTIND - 1))

mkdir -p testreports

sudo $($setup_script -d $container_path \
                -x $controller_bin \
                -a com.pelagicore.comptest)

eval `dbus-launch --sh-syntax`

./pam_stub.py &> pam.log &
pam_pid=$!
exit_codes=()

# Device node gateway tests
py.test test_devicenodegateway.py --junitxml=testreports/devicenodegateway.xml \
                                 --pelagicontain_binary ../../build/pelagicontain/src/pelagicontain \
                                 --container_path /tmp/container/
dng_exit=$?
exit_codes+=$dng_exit

# Pelagicontain tests
py.test test_pelagicontain.py --junitxml=testreports/pelagicontain.xml \
                              --pelagicontain_binary ../../build/pelagicontain/src/pelagicontain \
                              --container_path /tmp/container/

pelagicontain_exit=$?
exit_codes+=$pelagicontain_exit

# Network gateway tests
py.test test_networkgateway.py --junitxml=testreports/networkgateway.xml \
                               --pelagicontain_binary ../../build/pelagicontain/src/pelagicontain \
                               --container_path /tmp/container/

nwg_exit=$?
exit_codes+=$nwg_exit

# D-Bus gateway tests
py.test test_dbusgateway.py --junitxml=testreports/dbusgateway.xml \
                               --pelagicontain_binary ../../build/pelagicontain/src/pelagicontain \
                               --container_path /tmp/container/

dbusgw_exit=$?
exit_codes+=$dbusgw_exit

kill $pam_pid
wait $pam_pid 2>/dev/null

## Report
if [ "$dng_exit" == "0" ]; then
    echo "Device node manager: SUCCESS"
else
    echo "Device node manager: FAIL"
fi

if [ "$pelagicontain_exit" == "0" ]; then
    echo "Pelagicontain: SUCCESS"
else
    echo "Pelagicontain: FAIL"
fi

if [ "$nwg_exit" == "0" ]; then
    echo "Network Gateway: SUCCESS"
else
    echo "Network Gateway: FAIL"
fi

if [ "$dbusgw_exit" == "0" ]; then
    echo "D-Bus Gateway: SUCCESS"
else
    echo "D-Bus Gateway: FAIL"
fi

[[ $exit_codes =~ 1 ]] && exit 1 || exit 0
