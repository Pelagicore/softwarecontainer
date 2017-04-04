
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

import pytest

import subprocess
import os
import signal
import shutil
import time

from testframework import SoftwareContainerAgentHandler


# Add this directory (the package root) to the sys.path so the imports
# in the tests work, e.g. importing from 'testframework' to access helper
# classes etc.
PACKAGE_ROOT = os.path.dirname(os.path.abspath(__file__))
os.sys.path.insert(0, PACKAGE_ROOT)


@pytest.fixture(scope="class")
def dbus_launch():
    """ Setting up dbus environement variables for session bus """
    p = subprocess.Popen('dbus-launch', stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    for var in p.stdout:
        sp = var.split('=', 1)
        env_var = sp[0]
        env_val = sp[1][:-1]
        os.environ[env_var] = env_val
    yield
    # The DBUS_SESSION_BUS_PID is set to sp[1] in the last loop iteration above...
    os.kill(int(sp[1]), signal.SIGKILL)

@pytest.fixture(scope="class")
def assert_no_session_bus():
    """ Makes sure there is no session bus running in the environment for the test.

        This is done by backuping any existing dbus session vars in the environment, and then
        unsetting them. They are being reset in the teardown stage of this fixture.
    """
    address_var = "DBUS_SESSION_BUS_ADDRESS"
    pid_var = "DBUS_SESSION_BUS_PID"
    old_address = None
    old_pid = None
    if address_var in os.environ:
        old_address = os.environ[address_var]
        os.environ[address_var] = None

    if pid_var in os.environ:
        old_pid = os.environ[pid_var]
        os.environ[pid_var] = None

    yield

    if old_address is not None:
        os.environ[address_var] = old_address
    if old_pid is not None:
        os.environ[pid_var] = old_pid

@pytest.fixture(scope="class")
def agent(request, agent_without_checks):
    """ Start the Agent, check that it can connect to dbus and doesn't die (for example, because of
        bad config or command-line args, and then return an interface to it.
    """

    # This is the only difference between the start of the agent and this, that we do some checking.
    agent_without_checks.check_connection()
    yield agent_without_checks

@pytest.fixture(scope="class")
def agent_without_checks(request):
    """ Start the Agent and return an interface to it.

        This fixture requires the test module to have a function named 'logfile_path'
        on the module level.

        This does not check if the agent gets connected to dbus, or if it dies because of bad
        configuration or command-line args. Use the "agent" fixture for that
    """
    # Introspect the consuming module for the logfile path. The scope of this
    # fixture means the module is received through the 'request' parameter
    # and thus the path can be defined on the module level.
    logfile_path = request.module.logfile_path()

    # Introspect the consuming class or module for the config file location. To avoid having to
    # write new modules for all config file changes, this fixture is on a class-level, so one can
    # give the config file on either class level OR module level, depending on one's needs.
    config_file = None
    config_file_location = None

    if request.cls.__dict__ is not None and "agent_config" in request.cls.__dict__:
        config_file = request.cls.agent_config()
    elif "agent_config" in request.module.__dict__:
        config_file = request.module.agent_config()

    if config_file is not None:
        config_file_location = config_file.path()
        config_parent_dir = os.path.dirname(config_file_location)

        if not os.path.exists(config_parent_dir):
            os.makedirs(config_parent_dir)

        with open(config_file_location, "w") as fh:
            fh.write(config_file.config_as_string())

    if "agent_exec_prefix" in request.module.__dict__:
        exec_prefix = request.module.agent_exec_prefix()
    else:
        exec_prefix = None

    # Introspect the consuming module for what service manifests to use.
    # TODO: Perhaps this should be optionally on the class-level as well?
    standard_manifest_location = None
    default_manifest_location = None

    if "service_manifests" in request.module.__dict__:
        manifests = request.module.service_manifests()
        for manifest in manifests:
            if manifest.is_default():
                default_manifest_location = manifest.location() + "/service-manifest.default.d/"
                if not os.path.exists(default_manifest_location):
                    os.makedirs(default_manifest_location)
                service_manifest_file = default_manifest_location + manifest.name()
            else:
                standard_manifest_location = manifest.location() + "/service-manifest.d/"
                if not os.path.exists(standard_manifest_location):
                    os.makedirs(standard_manifest_location)
                service_manifest_file = standard_manifest_location + manifest.name()

            with open(service_manifest_file, "w") as fh:
                fh.write(manifest.json_as_string())

    # Start the agent handler
    auto_check_connection = False
    agent_handler = SoftwareContainerAgentHandler(logfile_path,
                                                  config_file_location,
                                                  standard_manifest_location,
                                                  default_manifest_location,
                                                  auto_check_connection,
                                                  exec_prefix)
    # Give it some time to start
    time.sleep(1)

    # Return control to the test code
    yield agent_handler

    # Shutdown the agent as part of the teardown of this fixture.
    agent_handler.terminate()
    time.sleep(0.5)

@pytest.fixture(scope="module")
def testhelper(request):
    """ Copy the testhelper module to the location that the test mounts into
        the container. This is needed to have the helper available to execute
        inside the container through LaunchCommand to drive the tests.
    """
    path = request.module.mounted_path_in_host()
    shutil.copyfile(PACKAGE_ROOT + "/testframework/testhelper.py", path + "/testhelper.py")
    yield
    os.remove(path + "/testhelper.py")


def grep_for_dbus_proxy():
    """ Helper for 'assert_no_proxy' """
    return os.popen('ps -aux | grep dbus-proxy | grep -v "grep"').read()


@pytest.fixture(scope="function")
def assert_no_proxy():
    """ Assert that dbus-proxy is not running when we expect it to not be.

        Do the check both on setup and teardown
    """
    assert "dbus-proxy" not in grep_for_dbus_proxy(), "dbus-proxy is alive when it shouldn't be"
    yield
    assert "dbus-proxy" not in grep_for_dbus_proxy(), "dbus-proxy is alive when it shouldn't be"


@pytest.fixture(scope="module")
def create_testoutput_dir(request):
    """ Create a directory for storing test output and test files to support
        troubleshooting etc.
    """
    output_dir = request.module.output_dir()

    if not os.path.exists(output_dir):
        os.makedirs(output_dir)
