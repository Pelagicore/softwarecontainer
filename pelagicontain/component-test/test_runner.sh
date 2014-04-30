#!/bin/bash

# Parse command line arguments
pelagicontain_bin=../../build/pelagicontain/src/pelagicontain
container_path=/tmp/container/
controller_bin=../../build/controller/src/controller
setup_script=../../scripts/setup_environment.sh

# Hardcoded test output paths for e.g. logs
test_output_path=/tmp/pelagicontain-comptests/
test_reports_path=$test_output_path/test-reports/
pam_log_path=$test_output_path/pam_log/

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

mkdir -p $test_reports_path
mkdir -p $pam_log_path


sudo $($setup_script -d $container_path \
                -x $controller_bin \
                -a com.pelagicore.comptest)

eval `dbus-launch --sh-syntax`

./pam_stub.py &> $pam_log_path/pam.log &
pam_pid=$!
exit_codes=()

# Device node gateway tests
py.test test_devicenodegateway.py --junitxml=$test_reports_path/devicenodegateway.xml \
                                  --pelagicontain-binary $pelagicontain_bin \
                                  --container-path $container_path
dng_exit=$?
exit_codes+=$dng_exit

# Pelagicontain tests
py.test test_pelagicontain.py --junitxml=$test_reports_path/pelagicontain.xml \
                              --pelagicontain-binary $pelagicontain_bin \
                              --container-path $container_path

pelagicontain_exit=$?
exit_codes+=$pelagicontain_exit

# Network gateway tests
py.test test_networkgateway.py --junitxml=$test_reports_path/networkgateway.xml \
                               --pelagicontain-binary $pelagicontain_bin \
                               --container-path $container_path

nwg_exit=$?
exit_codes+=$nwg_exit

# D-Bus gateway tests
py.test test_dbusgateway.py --junitxml=$test_reports_path/dbusgateway.xml \
                            --pelagicontain-binary $pelagicontain_bin \
                            --container-path $container_path

dbusgw_exit=$?
exit_codes+=$dbusgw_exit

# Pulseaudio gateway tests
py.test test_pulsegateway.py --junitxml=$test_reports_path/pulsegateway.xml \
                             --pelagicontain-binary $pelagicontain_bin \
                             --container-path $container_path

pulsegw_exit=$?
exit_codes+=$pulsegw_exit

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

if [ "$pulsegw_exit" == "0" ]; then
    echo "PulseAudio Gateway: SUCCESS"
else
    echo "PulseAudio Gateway: FAIL"
fi

[[ $exit_codes =~ 1 ]] && exit 1 || exit 0
