
# Copyright (C) 2016 Pelagicore AB
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


""" Introduction
    ============
    This test suite is used to measure timing values between different parts of
    softwarecontainer, both inside the core application, and outside it. The suite sets
    up all the necessary components to interact with softwarecontainer-agent, and issues
    commands to it using dbus.


    Goals
    =====
    * Run softwarecontainer-agent and get profiling values from it into a log file
    * Run several apps inside containers using softwarecontainer-agent and get profiling
    values from these runs.
    * Terminate everything and get profiling values from this.
"""

import pytest

import time
import os
import tempfile
import string
import re

from testframework import Container


CURRENT_DIR = os.path.dirname(os.path.realpath(__file__))


# This function is used by the 'agent' fixture to know where the log should be stored
def logfile_path():
    return CURRENT_DIR + "/test.log"


def run_test(num_starts=3):
        apps = list()
        for app in range(0, num_starts):
            """ Start numStarts apps in softwarecontainer.
            """
            print "Start app " + str(app)
            container = Container()

            # A basic container configuration, content is not important for this test.
            container_data = {
                Container.CONFIG: '[{"enableWriteBuffer": false}]',
                Container.BIND_MOUNT_DIR: "app",
                Container.HOST_PATH: CURRENT_DIR
            }
            container.start(container_data)

            # A minimal gateway config so the gateway can be configured and enabled.
            dbus_gw_config = [{
                "dbus-gateway-config-session": [{
                    "direction": "*",
                    "interface": "*",
                    "object-path": "*",
                    "method": "*"
                }],
                "dbus-gateway-config-system": []
            }]
            container.set_gateway_config("dbus", dbus_gw_config)

            container.launch_command("/gateways/app/simple")
            apps.append(container)

        time.sleep(5)

        # Tear down all started apps
        for app in apps:
            app.terminate()


def remove_ansi(astring):
    ansi_escape = re.compile(r'\x1b[^m]*m')
    return ansi_escape.sub('', astring)


def get_python_point(log_file, point_name, match_number=1):
    """ This function will grab the timestamp from a python log entry with pointName in
        the log_file
    """
    match = 0
    log_file.seek(0)
    for line in log_file:
        if re.search(point_name, line):
            match = match + 1
            if match == match_number:
                return remove_ansi(string.split(line)[2])
    print "Nothing found! " + point_name + " " + str(match_number)
    return 0


def get_function_log(log_file, function_name, match_number=1):
    match = 0
    log_file.seek(0)
    for line in log_file:
        if re.search(function_name + " end", line):
            match = match + 1
            if match == match_number:
                print line
                return remove_ansi(string.split(line)[6])
    print "Nothing found! " + function_name + " " + str(match_number)
    return 0


def get_log_point(log_file, point_name, match_number=1):
    """ This funciton will grab a timestamp from log_file with the entry name point_name, if no
        match_number is declared, it will return the first found entry.

        match_number can be used to set which entry to pick up, when multiple entries are found.
        This might useful when starting multiple apps for example.
    """
    match = 0
    log_file.seek(0)
    for line in log_file:
        if point_name in line:
            match = match + 1
            if match == match_number:
                return remove_ansi(string.split(line)[4])
    print "Nothing found! " + point_name + " " + str(match_number)
    return 0


def write_measurement(file_name, value, url=None):
    """ Write a measurement file to file_name containing the value and url entry.
        This is for the properties files as defined in Jenkins Plot Plugin
    """
    f = open(file_name, 'w')
    f.write("YVALUE=" + str(value) + "\n")
    if url is not None:
        f.write("URL=" + url + "\n")
    f.close()


def measure(log_file):
    if log_file is None:
        return False

    for line in log_file:
        print line

    start = "softwareContainerStart"
    end = "dbusAvailable"
    write_measurement("result-" + start + "-" + end + "-1.properties",
                      float(get_python_point(log_file, end)) - float(get_log_point(log_file, start)))

    for i in range(1, 4):
        start = "createContainerStart"
        end = "launchCommandEnd"
        write_measurement("result-" + start + "-" + end + "-" + str(i) + ".properties",
                          float(get_log_point(log_file, end, i)) - float(get_log_point(log_file, start, i)))

    profile_points = ["createContainerFunction",
                      "bindMountFolderInContainerFunction",
                      "setGatewayConfigsFunction",
                      "launchCommandFunction",
                      "shutdownContainerFunction"]
    for value in profile_points:
        write_measurement("result-" + value + ".properties", float(get_function_log(log_file, value)))


@pytest.mark.usefixtures("agent")
class TestTimingProfiling(object):

    def test_start_profiling(self):
        run_test()

        time.sleep(1)

        # The agent fixture has set the Agent to log to the file specified by logfile_path() in this module.
        # Now we need the content of that file for analysis. In this setup the Agent helper still has the file
        # open and while the below seems to work, it is probably a nasty hack. Beware of this if the output
        # from this test suite becomes strange...
        log_file = open(logfile_path(), "r")
        log_file.seek(0)
        measure(log_file)
