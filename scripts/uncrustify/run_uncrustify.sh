#!/bin/bash

if [ "$#" != "2" ] || [ "$2" != "--i-have-no-fear" ]; then
    echo "This will modify your pelagicontain source tree IN PLACE. Make sure"
    echo "all your changes are committed (and pushed somewhere remotely)"
    echo "before running this script. Re-run with --i-have-no-fear when ready."
    echo "Example: $0 ~/pelagicontain_sources --i-have-no-fear"
    exit
fi

if [ ! -d $1 ]; then
    echo "$1 Does not exist"
fi
uncrustify -c uncrustify.cfg --no-backup $(find $1 -name "*.[ch]")
