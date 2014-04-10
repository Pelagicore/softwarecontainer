#!/usr/bin/env python

"""
    Copyright (C) 2014 Pelagicore AB
    All rights reserved.
"""
import sys
import json
import time
import re
import os
import pytest
import subprocess
import signal
import distutils.spawn
import shutil

from common import ComponentTestHelper

# PulseAudio server requires a home directory to be set
os.environ['HOME'] = "/root/"

# Prepare configurations
audio_enabled = { "audio": "true" }
pulse_enabled_config = { "pulseaudio": json.dumps(audio_enabled) }
audio_disabled = { "audio": "false" }
pulse_disabled_config = { "pulseaudio": json.dumps(audio_disabled) }

helper = ComponentTestHelper()
container_root = None
pa_server_pid = None
app_bin = None

# Check that sys.argv is usable, notify user otherwise. Exits on failure
def ensure_command_line_ok():
    if len(sys.argv) != 3:
        print "Proper invocation looks like this:"
        print "%s <pelagicontain path> <container path>" % sys.argv[0]
        sys.exit(1)

# Start a pulse server on the host system. Necessary as existing servers usually
# are not run as root
def start_pa_server():
    process = subprocess.Popen(['pulseaudio', '--exit-idle-time=-1'], shell=True, preexec_fn=os.setsid)
    print "Spawned PulseAudio server with PID " + str(process.pid)
    return process.pid

# Kill the pulse server
def kill_pa_server():
    global pa_server_pid
    os.killpg(pa_server_pid, signal.SIGTERM)

# Copy paplay from the host system
def find_and_copy_paplay():
    global app_bin
    wav = "/usr/share/sounds/alsa/Front_Right.wav"
    if not os.path.isfile(wav):
        print "Could not find sound file to test with"
        return False

    paplay = distutils.spawn.find_executable("paplay")
    if not paplay:
        print "Could not find 'paplay'. Is it installed?"
        return False

    print "Copying %s to %s" % (paplay, app_bin)
    try:
        shutil.copy(paplay, app_bin)
        shutil.copy(wav, app_bin)
    except IOError as e:
        return False
    return True

# Run before first test case
def setup_suite():
    global container_root, pa_server_pid, app_bin
    ensure_command_line_ok()
    container_root = sys.argv[2]
    app_bin = container_root + "/" + helper.app_uuid + "/bin/"

    if not find_and_copy_paplay():
        print "Problem copying paplay and/or wav-file"
        sys.exit(1)

    pa_server_pid = start_pa_server()

# Run before each test case
def setup_test_case(config):
    global container_root, app_bin
    pelagicontain_binary = sys.argv[1]

    helper.pam_iface.helper_set_configs(config)
    time.sleep(1)
    if not helper.start_pelagicontain2(pelagicontain_binary, container_root,
                                       "/controller/controller"):
        print "Failed to launch pelagicontain!"
        sys.exit(1)
    print "Found Pelagicontain on DBUS: " + \
        str(helper.find_pelagicontain_on_dbus())

# Run after each test
def teardown_test_case():
    print "UnregisterClient called: " + \
        str(helper.pam_iface.test_unregisterclient_called())
    helper.shutdown_pelagicontain()

# Run after last tests
def teardown_suite():
    os.system("rm " + app_bin + "paplay")
    os.system("rm " + app_bin + "Front_Right.wav")
    kill_pa_server()

def run_app():
    print "Found and run Launch on DBUS: " + \
        str(helper.find_and_run_Launch_on_pelagicontain_on_dbus())
    print "Register called: " + \
        str(helper.pam_iface.test_register_called())
    print "Update finished called: " + \
        str(helper.pam_iface.test_updatefinished_called())

def is_app_output_ok(expected):
    path = container_root + \
        "/com.pelagicore.comptest/shared/pulsegateway_test_output"
    try:
        with open(path) as f:
            lines = f.readlines()
            if not len(lines) == 1:
                print "Too few or too many lines in log file"
                return False
            else:
                if expected in lines[0]:
                    return True
    except Exception as e:
        print "Unable to read command output, output file couldn't be opened!"
        print str(e)
        return False

setup_suite()

# Create contained app
with open(app_bin + "containedapp", "w") as f:
    print "Overwriting containedapp..."
    f.write("""#!/bin/sh
    /appbin/paplay --volume=0 /appbin/Front_Right.wav
    echo $? > /appshared/pulsegateway_test_output
    env
    ls /gateways/
    """)
os.system("chmod 755 " + app_bin + "containedapp")
time.sleep(1)

TOTAL_NUMBER_OF_TESTS = 2
TESTS_PASSED = 0

# Test with sound enabled
print "\nRunning test 1/2"
setup_test_case(pulse_enabled_config)
run_app()
time.sleep(2)
if is_app_output_ok("0"):
    print "PASS: Audio enabled"
    TESTS_PASSED += 1
else:
    print "FAIL: Audio should be enabled but is not"
teardown_test_case()

# Test with sound disabled
print "\nRunning test 2/2"
setup_test_case(pulse_disabled_config)
run_app()
time.sleep(2)
if is_app_output_ok("1"):
    print "PASS: Audio disabled"
    TESTS_PASSED += 1
else:
    print "FAIL: Audio should be disabled but is not"
teardown_test_case()

teardown_suite()

print "SUMMARY: " + str(TESTS_PASSED) + " of " + str(TOTAL_NUMBER_OF_TESTS) + " tests passed"
