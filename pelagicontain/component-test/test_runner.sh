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

sudo $setup_script -d $container_path \
                   -x $controller_bin \
                   -a com.pelagicore.comptest

eval `dbus-launch --sh-syntax`

./pam_stub.py &> $pam_log_path/pam.log &
pam_pid=$!
exit_codes=()
declare -A test_results

# Arguments: <Test name> <xml report filename> <testfile>
function run_pytest {
    py.test $3 --junitxml=$test_reports_path/$2.xml \
               --pelagicontain-binary $pelagicontain_bin \
               --container-path $container_path
    exitval=$?
    test_results["$1"]="$exitval"
    exit_codes+="$exitval"
}

# Run the tests
run_pytest 'Kill application' killapp test_kill.py
run_pytest 'Clean shutdown' cleanshutdown test_cleanshutdown.py
run_pytest 'Device node gateway' devicenodegateway test_devicenodegateway.py
run_pytest 'Pelagicontain' pelagicontain test_pelagicontain.py
run_pytest 'Network gateway' networkgateway test_networkgateway.py
run_pytest 'D-Bus gateway' dbusgateway test_dbusgateway.py
run_pytest 'Pulseaudio gateway' pulsegateway test_pulsegateway.py

kill $pam_pid
wait $pam_pid 2>/dev/null

## Report, unfortunately out of order here.
for key in "${!test_results[@]}";
do
    test_exit=${test_results["$key"]}
    if [ "$test_exit" == "0" ]; then
        echo "$key: SUCCESS"
    else
        echo "$key: FAIL"
    fi
done

[[ $exit_codes =~ 1 ]] && exit 1 || exit 0
