#!/bin/sh

# This is an example of a misbehaving application being launched in a
# container, the example will launch simple.c in a container, and observe that
# it is killed inside the container by the OOM killer.

# Register the path of the script. This is what we will mount in to the
# container later, in order to access the 'simple' binary.
SCRIPTPATH=$( cd $(dirname $0) ; pwd -P )

echo "Using system bus -- configure this in $0"
BUS="system"
# Uncomment the line below to use the session bus
#BUS="session"

# Create a new session D-Bus bus
eval `dbus-launch --sh-syntax`

# Print the environment
env

# Launch a new agent
pelagicontain-agent &
AGENTPID="$!"

# Let the agent start up
sleep 2

export PCCMD="dbus-send --${BUS} --print-reply --dest=com.pelagicore.PelagicontainAgent /com/pelagicore/PelagicontainAgent"

# Introspect the agent
$PCCMD org.freedesktop.DBus.Introspectable.Introspect

# Ping the agent
$PCCMD com.pelagicore.PelagicontainAgent.Ping

# Create a new container
$PCCMD com.pelagicore.PelagicontainAgent.CreateContainer string:prefix

# Expose a directory to the container
$PCCMD com.pelagicore.PelagicontainAgent.BindMountFolderInContainer uint32:0 string:${SCRIPTPATH} string:app boolean:true

# Run the simple example
$PCCMD com.pelagicore.PelagicontainAgent.LaunchCommand uint32:0 uint32:0 string:/gateways/app/simple string:/gateways/app/ string:/tmp/stdout dict:string:string:""

# Let the example run for a while
sleep 30

# Clean up
kill $AGENTPID
