
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

# External utility imports
import os

# Test framework and local test helper imports
from testframework import Capability
from testframework import StandardManifest
from testframework import Container

# Useful globals

CURRENT_DIR = os.path.dirname(os.path.realpath(__file__))
TESTOUTPUT_DIR = CURRENT_DIR + "/testoutput"

# The path to where the test app is located:
#  * Will be passed in the 'data' dict when starting the container to set
#    this test modules location as a bind mount visible inside the container.
#  * Is used by the testframework for knowing where this test is located.
HOST_PATH = os.path.dirname(os.path.abspath(__file__))


# Provide what the testframework requires #####
# This function is used by the test framework to know where test specific files should be stored
def output_dir():
    return TESTOUTPUT_DIR


# This function is used by the 'agent' fixture to know where the log should be stored
def logfile_path():
    return TESTOUTPUT_DIR + "/devicenode-test.log"


# This function is used by the testframework 'testhelper' fixture to know where the
# testhelper should be made available when running the tests
def mounted_path_in_host():
    return HOST_PATH

# Globals for setup and configuration of SC #####

# These default values are used to pass various test specific values and
# configurations to the Container helper. Tests that need to add, remove or
# update entries can simply base their dict on this one for convenience.
DATA = {
    Container.CONFIG: "[{\"writeBufferEnabled\": false}]",
    Container.BIND_MOUNT_DIR: "/gateways/app",
    Container.HOST_PATH: HOST_PATH,
    Container.READONLY: False
}


GW_CONFIG_SAME_DEV_0 = [
    {
        "name": "/dev/input/mouse0",
        "mode": 644
    }
]

GW_CONFIG_SAME_DEV_1 = [
    {
        "name": "/dev/input/mouse0",
        "mode": 754
    }
]

GW_CONFIG_NEW_DEV_0 = [
    {
        "name": "/dev/tty0",
        "mode": 644
    }
]

GW_CONFIG_NEW_DEV_1 = [
    {
        "name": "/dev/tty1",
        "mode": 755
    }
]

GW_CONFIG_SAME_DEV_SAME_MODE = [
    {
        "name": "/dev/tty0",
        "mode": 644
    }
]

""" Following capabilities are fed to DynamicConfiguration tests to examine the case
    when new mode is configured for same device.
"""
test_cap_0 = Capability("double.activation.same.device.0.cap",
                        [
                            {"id": "devicenode", "config": GW_CONFIG_SAME_DEV_0}
                        ])

test_cap_1 = Capability("double.activation.same.device.1.cap",
                        [
                            {"id": "devicenode", "config": GW_CONFIG_SAME_DEV_1}
                        ])

""" Following capabilities are fed to DynamicConfiguration tests to examine the case
    when a new device is added to configuration
"""
test_cap_2 = Capability("double.activation.new.device.0.cap",
                        [
                            {"id": "devicenode", "config": GW_CONFIG_NEW_DEV_0}
                        ])

test_cap_3 = Capability("double.activation.new.device.1.cap",
                        [
                            {"id": "devicenode", "config": GW_CONFIG_NEW_DEV_1}
                        ])

""" Following capability is fed to DynamicConfiguration tests to examine the case
    when exactly same device with same mode is added
"""
test_cap_4 = Capability("double.activation.same.device.same.mode.cap",
                        [
                            {"id": "devicenode", "config": GW_CONFIG_SAME_DEV_SAME_MODE}
                        ])

""" Define the manifest so it can be used by the testframework  """
manifest = StandardManifest(TESTOUTPUT_DIR,
                            "devicenode-test-manifest.json",
                            [
                                test_cap_0,
                                test_cap_1,
                                test_cap_2,
                                test_cap_3,
                                test_cap_4
                            ])


def service_manifests():
    """ The agent fixture calls this function when it creates the service manifests
        that should be used with this test module. The agent fixture expects a list
        of StandardManifest and/or DefaultManifest objects.
    """
    return [manifest]

# Test suites


@pytest.mark.usefixtures("testhelper", "agent")
class TestDeviceNode(object):
    """ This suite tests the behavior of DeviceNodeGateway within given set of capabilities.

        The tests use the Agent D-Bus interface to enable gateway configurations,
        for a specified list of capabilities.
    """

    """ To see differences and aims of parameters please see comments on capability declaration
    """
    @pytest.mark.parametrize(("firstcap", "secondcap"), [
        ("double.activation.same.device.0.cap", "double.activation.same.device.1.cap"),
        ("double.activation.new.device.0.cap", "double.activation.new.device.1.cap"),
        ("double.activation.same.device.same.mode.cap", "double.activation.same.device.same.mode.cap")
    ])
    def test_dynamic_configuration(self, firstcap, secondcap):
        """ This function tests multiple activation of DeviceNodeGateway within given set
            of capabilities.

            The test is excpected to pass if there is no exception thwrown, otherwise it will fail.
        """
        try:
            sc = Container()
            sc.start(DATA)

            sc.set_capabilities([firstcap])
            sc.set_capabilities([secondcap])
        finally:
            sc.terminate()
