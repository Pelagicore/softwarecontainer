# Copyright (C) 2017 Pelagicore AB
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
import subprocess
import lxc

from enum import Enum

from testframework import Container
from testframework import SoftwareContainerAgentHandler
from testframework import ConfigFile


CURRENT_DIR = os.path.dirname(os.path.realpath(__file__))
TESTOUTPUT_DIR = CURRENT_DIR + "/testoutput"

# Provide what the testframework requires #####
# This function is used by the test framework to know where test specific files should be stored
def output_dir():
    return TESTOUTPUT_DIR


# This function is used by the 'agent' fixture to know where the log should be stored
def logfile_path():
    return TESTOUTPUT_DIR + "/test.log"


# This function is used by the testframework 'testhelper' fixture to know where the
# testhelper should be made available when running the tests
def mounted_path_in_host():
    return CURRENT_DIR

DATA = {
    Container.CONFIG: '[{"writeBufferEnabled": false}]',
    Container.BIND_MOUNT_DIR: "/gateways/app",
    Container.HOST_PATH: CURRENT_DIR,
    Container.READONLY: False
}

### Test suites ###

@pytest.mark.usefixtures("create_testoutput_dir", "assert_no_session_bus", "agent_without_checks")
class TestNoSessionBus(object):
    """ This test suite tests that SC properly fails to connect
        to the session bus when there is no such bus
    """

    @staticmethod
    def agent_config():
        """ The agent fixture calls this function when it creates the config file to be
            used in this class. It expects a ConfigFile object.
        """
        return ConfigFile(TESTOUTPUT_DIR + "/agent-config",
                {
                    "SoftwareContainer": {"use-session-bus": "true"}
                }
        )

    def test_session_bus_fails(self, agent_without_checks):
        """ The agent should fail directly, given the above config file
        """
        time.sleep(0.5)
        assert not agent_without_checks.is_alive()

@pytest.mark.usefixtures("create_testoutput_dir")
class TestClearOldContainers(object):
    """ This test suite tests that SC cleans out old container, i.e.
        container related files left on the file system.
    """

    def __create_lxc_container(self, container_name):
        c = lxc.Container(container_name)
        if c.defined or (not c.create("busybox", lxc.LXC_CREATE_QUIET)):
            raise
        else:
            return c

    def __destroy_lxc_container(self, container):
        if container.running:
            if not container.shutdown(1) and not container.stop():
                raise
        if not container.destroy():
            raise

    class CON_TYPE(Enum):
        SC = 'SC'
        OTHER = 'OTHER'

    @pytest.mark.parametrize("containers",[
        # Only a bunch of SC containers
        [CON_TYPE.SC],
        [CON_TYPE.SC, CON_TYPE.SC],
        [CON_TYPE.SC, CON_TYPE.SC, CON_TYPE.SC],
        # Only a bunch of regular LXC containers
        [CON_TYPE.OTHER],
        [CON_TYPE.OTHER, CON_TYPE.OTHER],
        [CON_TYPE.OTHER, CON_TYPE.OTHER, CON_TYPE.OTHER],
        # Combinations of SC and regular LXC containers
        [CON_TYPE.OTHER, CON_TYPE.SC, CON_TYPE.SC],
        [CON_TYPE.SC, CON_TYPE.OTHER, CON_TYPE.SC],
        [CON_TYPE.SC, CON_TYPE.SC, CON_TYPE.OTHER],
        [CON_TYPE.OTHER, CON_TYPE.OTHER, CON_TYPE.SC],
        [CON_TYPE.OTHER, CON_TYPE.SC, CON_TYPE.OTHER],
        [CON_TYPE.SC, CON_TYPE.OTHER, CON_TYPE.OTHER]
    ])
    def test_old_containers_cleanup(self, containers):
        """ We want to test that old SC containers, but no other containers,
            are destroyed on startup of the agent. Since we have to start and
            kill the agent manually here, we don't use the agent fixture.
        """

        # Start the agent and let it start up
        agent_handler = SoftwareContainerAgentHandler(logfile_path(), None, None)
        time.sleep(0.5)

        other_name = "agent-servicetest"
        other_counter = 0
        sc_containers = dict()
        other_containers = []

        try:
            for contype in containers:
                # Create an SC container
                if contype == self.CON_TYPE.SC:
                    sc = Container()
                    container_id = sc.start(DATA)
                    container_name = "SC-" + str(container_id)
                    sc_containers[container_name] = sc
                # Create just any LXC container
                else:
                    skipped_container = self.__create_lxc_container(other_name + str(other_counter))
                    other_counter += 1
                    other_containers.append(skipped_container)

            # Kill SC-agent without cleanup
            agent_handler.terminate(kill=True)
            time.sleep(0.1)

            existing_containers = lxc.list_containers()
            assert len(existing_containers) == len(sc_containers) + len(other_containers)

            # Start the agent and give it time to kill the created container
            agent_handler = SoftwareContainerAgentHandler(logfile_path(), None, None)
            time.sleep(0.5)

            # Check the content of lxc-ls
            existing_containers = lxc.list_containers()
            assert len(existing_containers) == len(other_containers)

        finally:
            # Exit and cleanup
            agent_handler.terminate()

            # Remove all other LXC containers
            for cont in other_containers:
                self.__destroy_lxc_container(cont)

