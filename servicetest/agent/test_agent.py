
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
# import lxc

from testframework import Container, SoftwareContainerAgentHandler


CURRENT_DIR = os.path.dirname(os.path.realpath(__file__))
TESTOUTPUT_DIR = CURRENT_DIR + "/testoutput"

# Provide what the testframework requires #####

# This function is used by the 'agent' fixture to know where the log should be stored
def logfile_path():
    return TESTOUTPUT_DIR + "/test.log"


# This function is used by the testframework 'testhelper' fixture to know where the
# testhelper should be made available when running the tests
def mounted_path_in_host():
    return CURRENT_DIR

DATA = {
    Container.CONFIG: '[{"enableWriteBuffer": false}]',
    Container.BIND_MOUNT_DIR: "/gateways/app",
    Container.HOST_PATH: CURRENT_DIR,
    Container.READONLY: False
}

@pytest.fixture
def create_testoutput_dir(scope="module"):
    """ Create a directory for the generated test files.

        This directory is ignored by git but it's nice to have
        somewhere locally to store test output to support
        troubleshooting etc.
    """
    if not os.path.exists(TESTOUTPUT_DIR):
        os.makedirs(TESTOUTPUT_DIR)

# Test suites #


@pytest.mark.usefixtures("create_testoutput_dir", "agent")
class TestClearOldContainers(object):
    """ This test suite tests that SC cleans out old container, i.e.
        container related files left on the file system.
    """

    def test_old_containers_cleanup(self):
        """ The first goal of this test is to create a sitation where files from
            a previous run of SC are left on the file system.
            To be able to produce it a SC is started and after that
            softwarecontainer-agent is killed.

            The second phase of the test is to run SC again and make sure
            it does not fail because of any files left from the previous run.
        """
        sc = Container()
        container_id = sc.start(DATA)
        os.system("pkill --signal SIGKILL -f softwarecontainer-agent")
        time.sleep(0.5)

        os.system("lxc-ls >> output")
        with open("output", "r") as fh:
            file_content = fh.readline()
        file_content = file_content.strip()
        assert file_content == ("SC-" + str(container_id))
        os.remove("output")

        agent_handler = SoftwareContainerAgentHandler(logfile_path(), None, None)
        time.sleep(0.5)
        os.system("lxc-ls >> output")
        with open("output", "r") as fh:
            file_content = fh.readline()
        assert file_content == ""
        agent_handler.terminate()
        os.remove("output")
