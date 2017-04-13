#!/bin/sh

# Copyright (C) 2016-2017 Pelagicore AB
#
# Permission to use, copy, modify, and/or distribute this software for
# any purpose with or without fee is hereby granted, provided that the
# above copyright notice and this permission notice appear in all copies.
#
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
# WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR
# BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES
# OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
# WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
# ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
# SOFTWARE.
#
# For further information see LICENSE
#

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

export PATH=$PATH:$SCRIPTPATH
if ! which temperatureservice; then
    echo "temperatureservice is not in your \$PATH"
    exit 1
fi

# Create a new session D-Bus bus
eval `dbus-launch --sh-syntax`

temperatureservice &
TEMPERATUREPID="$!"

# Launch a new agent
softwarecontainer-agent -m ${SCRIPTPATH}/ &
AGENTPID="$!"

# Let the agent start up
sleep 2

# Destination
SCNAME="com.pelagicore.SoftwareContainerAgent"
# Object path
SCOBJPATH="/com/pelagicore/SoftwareContainerAgent"
# Prefix for dbus methods
AGENTPREFIX="com.pelagicore.SoftwareContainerAgent"
SC_CMD="dbus-send --${BUS} --print-reply --dest=$SCNAME $SCOBJPATH"

# Create a new container
$SC_CMD $AGENTPREFIX.Create string:'[{"writeBufferEnabled": false}]'

# A few thing that we use for more or less every call below
CONTAINERID="int32:0"
ROOTID="uint32:0"
OUTFILE="/tmp/stdout"

# Expose a directory to the container
APPBASE="/gateways/app"
$SC_CMD $AGENTPREFIX.BindMount \
    $CONTAINERID \
    string:${SCRIPTPATH} \
    string:${APPBASE} \
    boolean:false

# Set the capabilities needed to get and set temperature
$SC_CMD $AGENTPREFIX.SetCapabilities \
    $CONTAINERID \
    array:string:"com.pelagicore.temperatureservice.settemperature","com.pelagicore.temperatureservice.gettemperature","com.pelagicore.sethome"

sleep 1

# Run the simple example
$SC_CMD $AGENTPREFIX.Execute \
    $CONTAINERID \
    string:$APPBASE/temperatureserviceconsoleclient \
    string:$APPBASE \
    string:$OUTFILE \
    dict:string:string:""

tail -f $OUTFILE &
TAILPID="$!"

# Let the example run for a while
sleep 10

kill $TAILPID
$SC_CMD $AGENTPREFIX.Destroy $CONTAINERID
sleep 1

# Clean up
kill $AGENTPID
kill $TEMPERATUREPID
