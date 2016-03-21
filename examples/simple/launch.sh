#!/bin/sh

# This is an example of a misbehaving application being launched in a
# container, the example will launch simple.c in a container, and observe that
# it is killed inside the container by the OOM killer.

# Create a new session D-Bus bus
eval `dbus-launch --sh-syntax`

# Print the environment
env

# Launch a new agent
../../build/agent/pelagicontain-agent &
AGENTPID="$!"

# Let the agent start up
sleep 2

export PCCMD="gdbus call --session -d com.pelagicore.PelagicontainAgent -o /com/pelagicore/PelagicontainAgent"

# Introspect the agent
gdbus introspect --session -d com.pelagicore.PelagicontainAgent -o /com/pelagicore/PelagicontainAgent

# Ping the agent
$PCCMD -m com.pelagicore.PelagicontainAgent.Ping

# Create a new container
$PCCMD -m com.pelagicore.PelagicontainAgent.CreateContainer prefix

# Expose a directory to the container
$PCCMD -m com.pelagicore.PelagicontainAgent.BindMountFolderInContainer 0 /home/vagrant/pelagicontain/examples/simple app true

# Run the simple example
$PCCMD -m com.pelagicore.PelagicontainAgent.LaunchCommand 0 0 /gateways/app/simple /gateways/app/ /tmp/stdout "{}"

# Let the example run for a while
sleep 30

# Clean up
kill $AGENTPID
