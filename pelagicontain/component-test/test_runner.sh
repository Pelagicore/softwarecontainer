#!/bin/bash

# Parse command line arguments
pelagicontain_bin=../../build/pelagicontain/src/pelagicontain 
container_path=/tmp/container/

while getopts p:c: opt; do
    case $opt in
    p)
        pelagicontain_bin=$OPTARG
        ;;
    c)
        container_path=$OPTARG
        ;;
    esac
done
shift $((OPTIND - 1))

./pam_stub.py &> pam.log &
pam_pid=$!

./test_devicenodegateway.py $pelagicontain_bin $container_path
dng_exit=$?

kill $pam_pid
wait $pam_pid 2>/dev/null

reset

## Report
if [ "$dng_exit" == "0" ]; then
    echo "Device node manager: SUCCESS"
else
    echo "Device node manager: FAIL"
fi
exit 0
