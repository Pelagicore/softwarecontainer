
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

import os
import time
import signal
import shutil

from testframework.testhelper import CoreDumpHelper
from testframework import Container
from testframework import Capability
from testframework import DefaultManifest

CURRENT_DIR = os.path.dirname(os.path.realpath(__file__))
TESTOUTPUT_DIR = CURRENT_DIR + "/testoutput"

# This function is used by the test framework to know where test specific files should be stored
def output_dir():
    return TESTOUTPUT_DIR

# This function is used by the 'agent' fixture to know where the log should be stored
def logfile_path():
    return TESTOUTPUT_DIR + "/test.log"

def coredump_path():
    return "/tmp/coredumps"

# These default values are used to pass various test specific values and
# configurations to the Container helper. Tests that need to add, remove or
# update entries can simply base their dict on this one for convenience.
DATA = {
    Container.CONFIG: '[{"writeBufferEnabled": false}]',
    Container.BIND_MOUNT_DIR: "/gateways/app",
    Container.HOST_PATH: CURRENT_DIR,
    Container.READONLY: False
}

# This function is used by the testframework 'testhelper' fixture to know where the
# testhelper should be made available when running the tests
def mounted_path_in_host():
    return CURRENT_DIR

#
# Gateway config and capability + service manifest for enabling core dumps
#
FILE_CONFIG = [{
    "path-host": coredump_path(),
    "path-container": coredump_path(),
    "read-only": False
}]
default_cap = Capability("test.cap.default.coredump",
                        [
                            { "id": "file", "config": FILE_CONFIG }
                        ])
default_manifest = DefaultManifest(TESTOUTPUT_DIR, "default-caps.json", [default_cap])

def service_manifests():
    """ The agent fixture calls this function when it creates the service manifests
        that should be used with this test module. The agent fixture expects a list
        of StandardManifest and/or DefaultManifest objects.
    """
    return [default_manifest]


@pytest.fixture
def create_coredump_dir(scope="module"):
    """ Create the directories needed for getting the core dumps
    """

    # Create the core dump directory if it doesn't already exist
    if not os.path.exists(coredump_path()):
        os.makedirs(coredump_path())

    yield

    shutil.rmtree(coredump_path())
    assert(not os.path.exists(coredump_path()))

@pytest.fixture
def core_pattern(scope="function"):
    """ Save the old core_pattern, set it to something for this test
        and then reset the pattern to the old one during teardown.
    """

    # This is where the pattern is located
    pattern_path = "/proc/sys/kernel/core_pattern"
    # This is the naming we use during the test
    test_pattern = coredump_path() + "/testdump-%e-%t"
    old_pattern = ""

    with open(pattern_path, "r") as core_file:
        old_pattern = core_file.read().strip()

    with open(pattern_path, "w") as core_file:
        core_file.write(test_pattern)

    yield test_pattern

    # Teardown
    with open(pattern_path, "w") as core_file:
        core_file.write(old_pattern)

@pytest.mark.usefixtures("testhelper", "dbus_launch", "create_testoutput_dir",
                         "create_coredump_dir", "agent", "assert_no_proxy")
class TestCore(object):
    """ This suite tests that core dumps are created properly.

        The idea is to have a default capability that mounts whatever directory one
        wants to have available for core dumps into the container, and to make sure
        they end up there.

        We run a test helper that checks some core dump related options, then exits
        with a core dump signal (SIGABRT), then we check that 1) a dump was created,
        2) the checks inside the container match what we set up outside.
    """

    def test_core_is_dumped(self, core_pattern):
        """ Test that the core is dumped properly
        """
        sc = Container()

        try:
            sc.start(DATA)

            # Start the helper that creates a core dump
            # This helper will call os.abort() on itself, which should generate a core dump
            pid = sc.launch_command("python " +
                              sc.get_bind_dir() +
                              "/testhelper.py --test-dir " +
                              sc.get_bind_dir() + "/testoutput" +
                              " --core-dump " + coredump_path()
                              )

            # Let the helper run a little
            time.sleep(0.1)

            # Check that a core was dumped, meaning there are more files in the core dump dir now
            coreFiles = os.listdir(coredump_path())
            assert(len(coreFiles) == 1)

            # The core file should not have size 0
            fileStat = os.stat(coredump_path() + "/" + coreFiles[0])
            assert(fileStat.st_size > 0)

            # The values that the helper found inside the container are verified
            helper = CoreDumpHelper(TESTOUTPUT_DIR);
            assert(helper.patternResult().strip() == core_pattern)
            assert(helper.limitResult() != 0)

        finally:
            sc.terminate()
