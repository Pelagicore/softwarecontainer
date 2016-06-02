#!/bin/bash

logName="dlt.log"

measure() {
    startName=$1
    endName=$2

    start=`cat $logName |grep profilerPoint | grep $startName |tr -s '[[:space:]]' | cut -d " " -f 14|tr -d "]"`
    end=`cat $logName |grep profilerPoint | grep $endName |tr -s '[[:space:]]' | cut -d " " -f 14|tr -d "]"`

    retval=`echo "$end-$start" |bc`
}

# Setup environment for tests
echo "### Starting dlt-daemon ###"
dlt-daemon &
dpid=$!

echo "### Starting dlt-receive ###"
export LD_LIBRARY_PATH=$(dirname $(dirname $(which dlt-receive)))/lib
dlt-receive -a localhost 1> $logName &
rpid=$!

# Run the actual test cases
./launch.sh

# Shutdown the environment used by the tests
if ! kill $rpid > /dev/null 2>&1 ; then
    echo "Failed to kill dlt-receiver"
fi

if ! kill $dpid > /dev/null 2>&1 ; then
    echo "Failed to kill dlt-daemon"
fi

# Final measurements and output in reasonable format
echo "### Final measurements ###"
declare -A arr
arr["createContainerStart"]=launchCommandEnd
arr["softwareContainerStart"]=createContainerStart

for key in ${!arr[@]}; do
    measure ${key} ${arr[${key}]}
    # TODO Something useful by jenkins?
    echo "${key} ${arr[${key}]} => $retval"
done

# Grab information and measure it
