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
    exit
}


BUS="system"
while getopts ":b:h" opt; do
    case $opt in 
    b)
        BUS="$OPTARG"
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

echo "### CreateContainer ###"
# Create a new container
containerId=$($PCCMD com.pelagicore.PelagicontainAgent.CreateContainer string:prefix | while read row; do echo $row|grep uint32|cut -d " " -f 2; done)

echo "### BindMountFolderInContainer ###"
# Expose a directory to the container
$PCCMD com.pelagicore.PelagicontainAgent.BindMountFolderInContainer uint32:0 string:${SCRIPTPATH} string:app boolean:true

echo "### LaunchCommand ###"
# Run the simple example
$PCCMD com.pelagicore.PelagicontainAgent.LaunchCommand uint32:0 uint32:0 string:/gateways/app/simple string:/gateways/app/ string:/tmp/stdout dict:string:string:""

# Let the example run for a while
sleep 30

echo "### LaunchCommand ###"
# Run the simple example
$PCCMD com.pelagicore.PelagicontainAgent.ShutDownContainer uint32:$containerId

# Clean up
kill $AGENTPID

kill $DBUS_SESSION_BUS_PID
