#!/usr/bin/env python

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

#
# Don't run this from the scripts/ directory. It is copied into the build dir
# by CMake, and it is there it should be run.
#

import os
import sys
import stat
import shutil
import tempfile
import subprocess

from distutils.spawn import find_executable
from os.path import dirname

CURRENT_DIR = os.path.dirname(os.path.realpath(__file__))
os.chdir(CURRENT_DIR)

if "UID" in os.environ and os.environ["UID"] != 0:
    print "This script must be run as root"
    sys.exit(1)

processes = { }
def startProcess(binary, name):
    print "### Launching {} ###".format(name)
    try:
        process = subprocess.Popen(binary, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
        processes[name] = process
        return process
    except OSError as err:
        print "Got error: {} when starting {}".format(err, binary)
        sys.exit(1)

# Cleanup
def cleanup():
    for name, process in processes.items():
        print "### Killing {} at pid {} ###".format(name, process.pid)
        process.terminate()

#
# Start DLT-daemon and DLT_receive
#
startProcess("dlt-daemon", "DLT daemon")
dltLibs = os.path.join(dirname(dirname(find_executable("dlt-receive"))), "lib")
os.environ["LD_LIBRARY_PATH"] = dltLibs
startProcess(["dlt-receive","-a","localhost"], "DLT receiver")

#
# Start a DBus Session
#
dbus = startProcess("dbus-launch", "DBus daemon")
for var in dbus.stdout:
    sp = var.split('=', 1)
    os.environ[sp[0]] = sp[1][:-1]

#
# Start a PulseAudio server
#
pulse_server="/tmp/pulse.sock"
startProcess(["pulseaudio","--daemonize","--exit-idle-time=-1"], "PulseAudio")
os.system("pactl load-module module-native-protocol-unix auth-anonymous=1 socket={}".format(pulse_server))
os.environ["PULSE_SERVER"] = pulse_server

#
# Start Weston, a wayland compositor
#
xdg_path = tempfile.mkdtemp()
os.chmod(xdg_path, stat.S_IRWXU | stat.S_IRUSR | stat.S_IWUSR | stat.S_IXUSR)
os.environ["XDG_RUNTIME_DIR"] = xdg_path
startProcess(["weston","--backend=headless-backend.so"], "Weston")

print "### Running tests ###"
gtestFilter = ""
if len(sys.argv) > 1:
    gtestFilter = "--gtest_filter={}".format(sys.argv[1])

testSuites = { }
def runTestSuite(binary, name):
    outputFile = "{}_componenttest_result.xml".format(name)
    if os.path.exists(outputFile):
        os.remove(outputFile)

    command = "{} {} --gtest_output=xml:{}".format(binary, gtestFilter, outputFile)
    exitCode = os.system(command)
    testSuites[name] = exitCode

#
# NOTE: Don't add spaces in the names, the names are used as parts of paths to save
#       test results.
#

# These are the component tests
runTestSuite("./agent/component-test/softwarecontaineragent-component-test", "AgentComponentTest")
runTestSuite("./libsoftwarecontainer/component-test/softwarecontainer-component-test", "LibComponentTest")

cleanup()
os.kill(int(os.environ["DBUS_SESSION_BUS_PID"]), 15)
shutil.rmtree(xdg_path);

retval = 0
for name,exitCode in testSuites.items():
    if exitCode is not 0:
        retval += exitCode
        red = "\033[1;31m"
        reset = "\033[0;0m"
        errorString = "*** The {} failed with exit code {} ***".format(name, exitCode)
        print "{}{}".format(red, "*" * len(errorString))
        print errorString
        print "{}{}".format("*" * len(errorString), reset)

sys.exit(retval)
