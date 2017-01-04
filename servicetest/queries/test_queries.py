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
    The purpose of this test suite is to verify queries, e.g. list functions.

"""

import pytest

import os
import time

from testframework import Container
from testframework import Capability
from testframework import StandardManifest


CURRENT_DIR = os.path.dirname(os.path.realpath(__file__))
TESTOUTPUT_DIR = CURRENT_DIR + "/testoutput"


# This function is used by the 'agent' fixture to know where the log should be stored
def logfile_path():
    return TESTOUTPUT_DIR + "/test.log"


# These default values are used to pass various test specific values and
# configurations to the Container helper. Tests that need to add, remove or
# update entries can simply base their dict on this one for convenience.
DATA = {
    Container.CONFIG: '[{"enableWriteBuffer": false}]',
    Container.BIND_MOUNT_DIR: "/gateways/app",
    Container.HOST_PATH: CURRENT_DIR,
    Container.READONLY: False
}


""" An empty capability for the service manifest """
test_cap = Capability("test.cap.valid-dbus",[])

manifest = StandardManifest(TESTOUTPUT_DIR,
                            "queries-test-manifest.json",
                            [test_cap])

def service_manifests():
    """ The agent fixture calls this function when it creates the service manifests
        that should be used with this test module. The agent fixture expects a list
        of StandardManifest and/or DefaultManifest objects.
    """
    return [manifest]

# This function is used by the testframework 'testhelper' fixture to know where the
# testhelper should be made available when running the tests
def mounted_path_in_host():
    return CURRENT_DIR

@pytest.fixture
def create_testoutput_dir(scope="module"):
    """ Create a directory for the generated test files.

        This directory is ignored by git but it's nice to have
        somewhere locally to store test output to support
        troubleshooting etc.
    """
    if not os.path.exists(TESTOUTPUT_DIR):
        os.makedirs(TESTOUTPUT_DIR)

##### Test suites #####

@pytest.mark.usefixtures("testhelper", "create_testoutput_dir", "agent")
class TestContainerQueries(object):
    """ This suite tests queries on SoftwareContainers.
    """

    def test_list_containers(self):
        """ Test listing available containers works as expected
        """
        sc1 = Container()
        sc2 = Container()
        try:
            container_id_1, success_1 = sc1.start(DATA)
            container_id_2, success_2 = sc2.start(DATA)

            assert success_1 == True
            assert success_2 == True

            containers = sc1.list_containers()
            # Both containers should be listed and contain the ID:s of two containers
            assert len(containers) == 2
            assert containers.count(container_id_1) == 1 and containers.count(container_id_2) == 1
        finally:
            sc1.terminate()
            sc2.terminate()

    def test_list_containers_with_terminate(self):
        """ Test listing available containers works as expected if no containers are started
            and if containers have been started and then terminated
        """

        sc1 = Container()
        sc2 = Container()
        sc3 = Container()
        try:
            # Verify the list of containers is empty before we start anything
            containers = sc1.list_containers()
            assert len(containers) == 0

            container_id_1, success_1 = sc1.start(DATA)
            container_id_2, success_2 = sc2.start(DATA)

            assert success_1 == True
            assert success_2 == True

            # Both containers should be listed and contain the ID:s of two containers
            containers = sc1.list_containers()
            assert len(containers) == 2
            assert containers.count(container_id_1) == 1 and containers.count(container_id_2) == 1

            # Remove one of the containers and verify that it is not still in the list
            # Start a third container at the same time and verify that it is in the list
            container_id_3, success_3 = sc3.start(DATA)
            sc1.terminate()
            containers = sc1.list_containers()
            assert len(containers) == 2
            assert containers.count(container_id_2) == 1 and containers.count(container_id_3) == 1
            assert not containers.count(container_id_1) == 1
        finally:
            sc1.terminate() # In case it didn't get terminated properly
            sc2.terminate()
            sc3.terminate()
