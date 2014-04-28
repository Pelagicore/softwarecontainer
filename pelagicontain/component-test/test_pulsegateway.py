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

# Prepare configurations
audio_enabled = { "audio": "true" }
audio_disabled = { "audio": "false" }

TEST_CONFIGS = [audio_enabled, audio_disabled]

helper = ComponentTestHelper()
pa_server_pid = None
app_bin = None


@pytest.fixture(scope="module")
def setup_suite(container_path):
    """ Setup fixture that is run once per test run (as opposed to once
        per test function). Sets the HOME environment variable for the
        PulseAudio server, copies paplay from the host system and creates
        the sound file that is to be played in the test.
    """
    global pa_server_pid, app_bin
    app_bin = container_path + "/" + helper.app_uuid + "/bin/"

    # PulseAudio server requires a home directory to be set
    os.environ['HOME'] = "/root/"

    if not find_and_copy_paplay():
        print "Problem copying paplay"
        sys.exit(1)

    # Create sound file filled with noise
    os.system("dd if=/dev/urandom of=" + app_bin + "/test.wav bs=1k count=1")

    pa_server_pid = start_pa_server()

@pytest.fixture(scope="function", params=TEST_CONFIGS)
def setup_test_case(request, container_path, pelagicontain_binary):
    """ Setup fixture that is run once per test function. Sets the config
        for the current test, starts Pelagicontain and calls create_app().
    """
    global app_bin

    helper.pam_iface().helper_set_configs({ "pulseaudio": json.dumps(request.param) })
    time.sleep(1)
    if not helper.start_pelagicontain(pelagicontain_binary, container_path,
                                      "/controller/controller", False):
        print "Failed to launch pelagicontain!"
        sys.exit(1)
    print "Found Pelagicontain on D-Bus: " + \
        str(helper.find_pelagicontain_on_dbus())
    time.sleep(2)
    create_app()
    return request.param

@pytest.fixture(scope="module")
def teardown_suite(request):
    """ Teardown for entire test run, i.e. run after all the tests
        have been executed.
        Removes the paplay binary and the sound file. Kills the
        PulseAudio server.
    """
    def remove_files():
        os.system("rm " + app_bin + "/paplay")
        os.system("rm " + app_bin + "/test.wav")
    def kill_server():
        kill_pa_server()
    request.addfinalizer(remove_files)
    request.addfinalizer(kill_server)

def start_pa_server():
    """ Start a pulse server on the host system. Necessary
        as existing servers usually are not run as root.
    """
    process = subprocess.Popen(['pulseaudio', '--exit-idle-time=-1'],
                               shell=True, preexec_fn=os.setsid)
    print "Spawned PulseAudio server with PID " + str(process.pid)
    return process.pid

# Kill the pulse server
def kill_pa_server():
    global pa_server_pid
    os.killpg(pa_server_pid, signal.SIGTERM)

# Copy paplay from the host system
def find_and_copy_paplay():
    global app_bin

    paplay = distutils.spawn.find_executable("paplay")
    if not paplay:
        print "Could not find 'paplay'. Is it installed?"
        return False

    print "Copying %s to %s" % (paplay, app_bin)
    try:
        shutil.copy(paplay, app_bin)
    except IOError as e:
        return False
    return True

def create_app():
    """ Create an app that plays a wave file located in /appbin/.
        Note that this will overwrite existing apps named "containedapp".
    """
    global app_bin
    with open(app_bin + "containedapp", "w") as f:
        print "Overwriting containedapp..."
        f.write("""#!/bin/sh
                    /appbin/paplay --raw --volume=0 /appbin/test.wav
                    echo $? > /appshared/pulsegateway_test_output
                """)
    os.system("chmod 755 " + app_bin + "containedapp")

def run_app():
    """ Run /appbin/containedapp.
    """
    print "Found and run Launch on DBUS: " + \
        str(helper.find_and_run_Launch_on_pelagicontain_on_dbus())
    print "Register called: " + \
        str(helper.pam_iface().test_register_called())
    print "Update finished called: " + \
        str(helper.pam_iface().test_updatefinished_called())

def is_app_output_ok(expected, container_path):
    """ Parses output and checks whether it is expected.
    """
    success = False
    path = container_path + \
        "/com.pelagicore.comptest/shared/pulsegateway_test_output"
    try:
        with open(path) as f:
            lines = f.readlines()
            if not len(lines) == 1:
                print "Too few or too many lines in log file"
            else:
                if expected in lines[0]:
                    success = True
    except Exception as e:
        print "Unable to read command output, output file couldn't be opened!"
        print str(e)

    return success


class TestPulseGateway():
    def test_sound_enable_disable(
            self, setup_suite, setup_test_case, container_path,
            teardown_fixture, teardown_suite):

        config = setup_test_case
        run_app()
        time.sleep(2)

        if "true" in config["audio"]:
            assert is_app_output_ok("0", container_path)
        else:
            assert is_app_output_ok("1", container_path)
        print "UnregisterClient called: " + \
            str(helper.pam_iface().test_unregisterclient_called())
        helper.shutdown_pelagicontain()
