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

# conftest contains some fixtures we use in the tests
import conftest

from common import ComponentTestHelper

# Prepare configurations
audio_enabled = {"audio": "true"}
audio_disabled = {"audio": "false"}

TEST_CONFIGS = [audio_enabled, audio_disabled]

# We keep the helper object module global so external fixtures can access it
helper = ComponentTestHelper()

pa_server_pid = None
app_bin = None

PAPLAY_APP = """
#!/bin/sh
/appbin/paplay --raw --volume=0 /appbin/test.wav
echo $? > /appshared/pulsegateway_test_output
"""


@pytest.fixture(scope="module")
def setup_suite(container_path):
    """ Setup fixture that is run once per test run (as opposed to once
        per test function). Sets the HOME environment variable for the
        PulseAudio server, copies paplay from the host system and creates
        the sound file that is to be played in the test.
    """
    global pa_server_pid, app_bin
    app_bin = container_path + "/" + helper.app_id() + "/bin/"

    # PulseAudio server requires a home directory to be set
    os.environ['HOME'] = "/root/"
    os.environ['XDG_RUNTIME_DIR'] = '/tmp/test'
    os.environ['PULSE_SERVER'] = '/tmp/test/pulse/native'

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

    helper.pam_iface().helper_set_configs({"pulseaudio": json.dumps(request.param)})
    time.sleep(1)
    if not helper.start_pelagicontain(pelagicontain_binary, container_path):
        print "Failed to launch pelagicontain!"
        sys.exit(1)
    create_app()

    try:
        os.remove(container_path + helper.app_id() + "/shared/pulsegateway_test_output")
    except OSError as e:
        pass
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
        global pa_server_pid
        os.killpg(pa_server_pid, signal.SIGTERM)
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
        f.write(PAPLAY_APP)
    os.system("chmod 755 " + app_bin + "containedapp")


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
    """ Tests the Pelagicontain PulseAudio gateway using different configurations.
        The tests require that a PulseAudio server is running as root on the host
        system and that the paplay program exists so that it can be copied to the
        container. The setup_suite fixture is responsible for setting this up.
    """

    def test_sound_enable_disable(
            self, setup_suite, setup_test_case, container_path,
            teardown_fixture, teardown_suite):
        """ Tests the PulseAudio gateway with configurations for sound enabled and
            sound disabled. The test will launch an app that runs paplay and writes
            the exit signal to file. Because the test parametrization occues in the
            setup_test_case fixture, the test relies on the fixture to inform the
            test about what configuration has been used.

            Asserts that the log output of the application only contains a "0" when
            audio should be enabled and that it only contains a "1" when audio should
            be disabled.

                * Get config from setup-fixture
                * Run the app
                * Assert that the app's exit code corresponds to the expected value

            NOTE: The application run from within the tests sets output volume to 0
            so it will not be possible to hear any sound. Also, the sound file played
            by the application is less than a second short.
        """

        config = setup_test_case
        helper.pelagicontain_iface().Launch(helper.app_id())
        time.sleep(1)

        if "true" in config["audio"]:
            assert is_app_output_ok("0", container_path)
        else:
            assert is_app_output_ok("1", container_path)
