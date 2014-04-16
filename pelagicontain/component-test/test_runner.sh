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

$($setup_script -d $container_path \
                -x $controller_bin \
                -a com.pelagicore.comptest)

eval `dbus-launch --sh-syntax`

./pam_stub.py &> pam.log &
pam_pid=$!
exit_codes=()

py.test test_devicenodegateway.py --junitxml=testreports/devicenodegateway.xml \
                                  --pelagicontain_binary ../../build/pelagicontain/src/pelagicontain \
                                  --container_path /tmp/container/
dng_exit=$?
exit_codes+=$dng_exit

py.test test_pelagicontain.py --junitxml=testreports/pelagicontain.xml \
                                  --pelagicontain_binary ../../build/pelagicontain/src/pelagicontain \
                                  --container_path /tmp/container/

pelagicontain_exit=$?
exit_codes+=$pelagicontain_exit

kill $pam_pid
wait $pam_pid 2>/dev/null

reset

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

[[ $exit_codes =~ 1 ]] && exit 1 || exit 0
