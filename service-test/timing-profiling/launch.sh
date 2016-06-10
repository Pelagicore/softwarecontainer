#!/bin/sh

# This is an example of a misbehaving application being launched in a
# container, the example will launch simple.c in a container, and observe that
# it is killed inside the container by the OOM killer.

# Register the path of the script. This is what we will mount in to the
# container later, in order to access the 'simple' binary.
SCRIPTPATH=$( cd $(dirname $0) ; pwd -P )

help() {
    echo "$0 [-b [system|session]]"
    echo ""
    echo "  -b    what type of dbus to use, session or system"
    echo "  -n    Number of times to launch app"
    exit
}


BUS="system"
APPS=1
while getopts ":b:n:h" opt; do
    case $opt in 
    b)
        BUS="$OPTARG"
        ;;
    n)
        APPS="$OPTARG"
        ;;
    h)
        help
        ;;
    *)
        echo "Unknown option $OPTARG"
        help
    esac
done

# Create a new session D-Bus bus
eval `dbus-launch --sh-syntax`

# Launch a new agent
pelagicontain-agent &
AGENTPID="$!"

# Let the agent start up
sleep 2

export PCCMD="dbus-send --${BUS} --print-reply --dest=com.pelagicore.PelagicontainAgent /com/pelagicore/PelagicontainAgent"

declare -a containerIds

for i in `seq 1 $APPS`; do 
    echo "### CreateContainer ###"
    # Create a new container
    containerIds[$i]=$($PCCMD com.pelagicore.PelagicontainAgent.CreateContainer string:prefix | while read row; do echo $row|grep uint32|cut -d " " -f 2; done)

    echo "### BindMountFolderInContainer ###"
    # Expose a directory to the container
    $PCCMD com.pelagicore.PelagicontainAgent.BindMountFolderInContainer uint32:${containerIds[$i]} string:${SCRIPTPATH} string:app boolean:true

    echo "### LaunchCommand ###"
    # Run the simple example
    $PCCMD com.pelagicore.PelagicontainAgent.LaunchCommand uint32:${containerIds[$i]} uint32:0 string:/gateways/app/simple string:/gateways/app/ string:/tmp/stdout dict:string:string:""
done 

# Let the example run for a while
sleep 30

for i in `seq 1 $APPS`; do
    echo "### ShutdownCommand ###"
    # Run the simple example
    $PCCMD com.pelagicore.PelagicontainAgent.ShutDownContainer uint32:${containerIds[$i]}
done

# Clean up
kill $AGENTPID

kill $DBUS_SESSION_BUS_PID
