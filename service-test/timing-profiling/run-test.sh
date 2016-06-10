#!/bin/bash

logName="dlt.log"
appStarts=3


measure() {
    startName=$1
    endName=$2
    pointsToFind=1
    if [ "$3" != "" ] ; then 
        pointsToFind=$3
    fi
    echo "pointsTofind: $pointsToFind"

# BUG: Use these if getting from dlt logs
#    start=`cat $logName |grep profilerPoint | grep $startName |tr -s '[[:space:]]' | cut -d " " -f 14|tr -d "]"`
#    end=`cat $logName |grep profilerPoint | grep $endName |tr -s '[[:space:]]' | cut -d " " -f 14|tr -d "]"`
    declare -a start
    declare -a end
    for i in `seq 1 $pointsToFind`; do 
        echo "run $i"
        start[$i]=`cat $logName |grep profilerPoint | grep $startName | sed "${i}q;d" |tr -s '[[:space:]]' | cut -d " " -f 5`
        end[$i]=`cat $logName |grep profilerPoint | grep $endName | sed "${i}q;d" | tr -s '[[:space:]]' | cut -d " " -f 5`
        retval=`echo "${end[$i]}-${start[$i]}" |bc`
        echo "YVALUE=$retval" > "result-${startName}-${endName}-${i}.properties"
        echo "${key} ${arr[${key}]} result number $i => $retval"
        echo "end $i points: $pointsToFind"
    done

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
./launch.sh -b system -n $appStarts | tee $logName

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

measure createContainerStart launchCommandEnd 3
measure softwareContainerStart createContainerStart 1

