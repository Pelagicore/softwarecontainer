#!/bin/bash

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

if [ $UID != 0 ]; then
    echo "This script must be run as root"
    exit 1
fi

# cd to the directory of this script.
cd "$(dirname "$0")"

# The reason for breaking alphabetic ordering is that the
# cgroups test suite  has intermittent failures that are not yet fixed
DIRECTORIES=(cgroups agent capabilities coredump dbus environment filesystem networkgateway queries suspend timingprofiling devicenode)

# We want to exit with bad status in case some test fail. Mostly so that any
# CI system will notice that not everything was good.
EXITSTATUS=0
for DIR in ${DIRECTORIES[@]}; do
        echo "Running service tests in $DIR"
        pushd $DIR > /dev/null
        py.test -v --junit-xml=../${DIR}_servicetest_result.xml
        (( EXITSTATUS += $? ))
        # Sleep to allow some time for teardown in previous suite
        # to have full effect before we run the next suite
        sleep 1
        popd > /dev/null
done
exit $EXITSTATUS
