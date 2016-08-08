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

# Print the environment
env

# Launch a new agent
softwarecontainer-agent &
AGENTPID="$!"

# Let the agent start up
sleep 2

export PCCMD="dbus-send --${BUS} --print-reply --dest=com.pelagicore.SoftwareContainerAgent /com/pelagicore/SoftwareContainerAgent"

# Introspect the agent
$PCCMD org.freedesktop.DBus.Introspectable.Introspect

# Ping the agent
$PCCMD com.pelagicore.SoftwareContainerAgent.Ping

# Create a new container
$PCCMD com.pelagicore.SoftwareContainerAgent.CreateContainer string:prefix

# Expose a directory to the container
$PCCMD com.pelagicore.SoftwareContainerAgent.BindMountFolderInContainer uint32:0 string:${SCRIPTPATH} string:app boolean:true

# Run the simple example
$PCCMD com.pelagicore.SoftwareContainerAgent.LaunchCommand uint32:0 uint32:0 string:/gateways/app/simple string:/gateways/app/ string:/tmp/stdout dict:string:string:""

# Let the example run for a while
sleep 30

# Clean up
kill $AGENTPID
