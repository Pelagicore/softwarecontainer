#!/bin/bash

logName="dlt.log"

measure() {
    startName=$1
    endName=$2

# BUG: Use these if getting from dlt logs
#    start=`cat $logName |grep profilerPoint | grep $startName |tr -s '[[:space:]]' | cut -d " " -f 14|tr -d "]"`
#    end=`cat $logName |grep profilerPoint | grep $endName |tr -s '[[:space:]]' | cut -d " " -f 14|tr -d "]"`
    start=`cat $logName |grep profilerPoint | grep $startName |tr -s '[[:space:]]' | cut -d " " -f 5`
    end=`cat $logName |grep profilerPoint | grep $endName |tr -s '[[:space:]]' | cut -d " " -f 5`

    retval=`echo "$end-$start" |bc`
}

# BUG: ivi-logging and dlt is not friends, when we need timing between 
# components, this will need to be fixed, for now, we grab data from console 
# output
#
# # Setup environment for tests
# echo "### Starting dlt-daemon ###"
# dlt-daemon &
# dpid=$!
# 
# echo "### Starting dlt-receive ###"
# export LD_LIBRARY_PATH=$(dirname $(dirname $(which dlt-receive)))/lib
# dlt-receive -a localhost 1> $logName &
# rpid=$!

# Run the actual test cases
echo "### Running launch.sh script ###"
./launch.sh -b system | tee $logName

# # Shutdown the environment used by the tests
# if ! kill $rpid > /dev/null 2>&1 ; then
#     echo "Failed to kill dlt-receiver"
# fi
# 
# if ! kill $dpid > /dev/null 2>&1 ; then
#     echo "Failed to kill dlt-daemon"
# fi

# Final measurements and output in reasonable format
echo "### Final measurements ###"
declare -A arr
arr["createContainerStart"]=launchCommandEnd
arr["softwareContainerStart"]=createContainerStart

for key in ${!arr[@]}; do
    measure ${key} ${arr[${key}]}
    # TODO Something useful by jenkins?
    echo "YVALUE=$retval" > "result-${key}-${arr[${key}]}.properties"
    echo "${key} ${arr[${key}]} => $retval"
done

