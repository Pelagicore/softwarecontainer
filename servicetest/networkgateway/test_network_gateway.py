
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

import pytest

import os
import time

from testframework.testhelper import NetworkHelper
from testframework import Container


CURRENT_DIR = os.path.dirname(os.path.realpath(__file__))


##### Provide what the testframework requires #####

# This function is used by the 'agent' fixture to know where the log should be stored
def logfile_path():
    return CURRENT_DIR + "/test.log"


# This function is used by the testframework 'testhelper' fixture to know where the
# testhelper should be made available when running the tests
def mounted_path_in_host():
    return CURRENT_DIR


##### Configs #####

# These default values are used to pass various test specific values and
# configurations to the Container helper. Tests that need to add, remove or
# update entries can simply base their dict on this one for convenience.
DATA = {
    Container.CONFIG: '[{"enableWriteBuffer": false}]',
    Container.BIND_MOUNT_DIR: "app",
    Container.HOST_PATH: CURRENT_DIR,
    Container.READONLY: False
}

GW_CONFIG_POLICY_DROP = [
    {
        "type": "OUTGOING",
        "priority": 1,
        "rules": [],
        "default": "DROP"
    }
]

GW_CONFIG_POLICY_ACCEPT = [
    {
        "type": "OUTGOING",
        "priority": 1,
        "rules": [],
        "default": "ACCEPT"
    }
]

GW_CONFIG_REJECT_EXAMPLE_COM = [
    {
        "type": "OUTGOING",
        "priority": 1,
        "rules": [{"host": "example.com", "target": "REJECT"}],
        "default": "ACCEPT"
    }
]

GW_CONFIG_REJECT_EXAMPLE_COM_INPUT = [
    {
        "type": "INCOMING",
        "priority": 1,
        "rules": [{"host": "example.com", "target": "REJECT"}],
        "default": "ACCEPT"
    }
]


##### Test suites #####

@pytest.mark.usefixtures("testhelper", "agent")
class TestNetworkRules(object):
    """ This suite tests that whether NetworkGateway can parse given configuration and applies it to be used
        in SoftwareContainer with expected results.

        timeouts are:
             2 seconds for expected connectivity
            10 for expected output discconnectivity
            15 for expected input connectivity

        these durations will be shorten with -i option when possible
    """
    def test_network_policy_drop(self):
        """ Test setting network default target to drop and tests the corresponding behavior
            which is being not possible to ping a hostname
        """
        try:
            sc = Container()
            sc.start(DATA)
            sc.set_gateway_config("network", GW_CONFIG_POLICY_DROP)
            sc.launch_command("python " +
                              sc.get_bind_dir() +
                              "/testhelper.py" +
                              " --test-dir " + sc.get_bind_dir() +
                              " --do-ping example.com")
            # Allow 'ping' to timeout (the currently used busybox ping can't have custom timeout)
            time.sleep(10)
            helper = NetworkHelper(CURRENT_DIR)
            is_pingable = helper.ping_result()
            assert is_pingable is False
            # TODO: Move cleanup to a fixture (in all tests)
            helper.remove_file()
        finally:
            sc.terminate()

    def test_network_policy_accept(self):
        """ Test setting network default target to accept and tests the corresponding behavior
            which is being possible to ping hostname
        """
        try:
            sc = Container()
            sc.start(DATA)
            sc.set_gateway_config("network", GW_CONFIG_POLICY_ACCEPT)
            sc.launch_command("python " +
                              sc.get_bind_dir() +
                              "/testhelper.py" +
                              " --test-dir " + sc.get_bind_dir() +
                              " --do-ping example.com")
            time.sleep(2)
            helper = NetworkHelper(CURRENT_DIR)
            is_pingable = helper.ping_result()
            assert is_pingable is True
            helper.remove_file()
        finally:
            sc.terminate()

    def test_reject_outgoing_traffic_to_specific_domain_and_accept_others(self):
        """ Tests two things for outgoing traffic:
            * Test that a specific domain is rejected based on a rule.
            * Test that the default target, which is to accept, is retained when the reject rule
              is set.
        """
        try:
            sc = Container()
            sc.start(DATA)
            sc.set_gateway_config("network", GW_CONFIG_REJECT_EXAMPLE_COM)
            sc.launch_command("python " +
                              sc.get_bind_dir() +
                              "/testhelper.py" +
                              " --test-dir " + sc.get_bind_dir() +
                              " --do-ping example.com")
            # Allow 'ping' to timeout (the currently used busybox ping can't have custom timeout)
            time.sleep(10)
            helper = NetworkHelper(CURRENT_DIR)
            is_pingable = helper.ping_result()
            assert is_pingable is False

            helper.remove_file()
            sc.launch_command("python " +
                              sc.get_bind_dir() +
                              "/testhelper.py" +
                              " --test-dir " + sc.get_bind_dir() +
                              " --do-ping IANA.org")
            time.sleep(2)
            is_pingable = helper.ping_result()
            assert is_pingable is True
            helper.remove_file()
        finally:
            sc.terminate()

    def test_reject_incoming_traffic_from_specific_domain_and_accept_others(self):
        """ Tests two things for incoming traffic:
            * Test that a specific domain is rejected based on a rule.
            * Test that the default target, which is to accept, is retained when the reject rule
              is set.
        """
        try:
            sc = Container()
            sc.start(DATA)
            sc.set_gateway_config("network", GW_CONFIG_REJECT_EXAMPLE_COM_INPUT)
            sc.launch_command("python " +
                              sc.get_bind_dir() +
                              "/testhelper.py" +
                              " --test-dir " + sc.get_bind_dir() +
                              " --do-ping IANA.org")
            time.sleep(2)
            helper = NetworkHelper(CURRENT_DIR)
            is_pingable = helper.ping_result()
            assert is_pingable is True

            helper.remove_file()
            sc.launch_command("python " +
                              sc.get_bind_dir() +
                              "/testhelper.py" +
                              " --test-dir " + sc.get_bind_dir() +
                              " --do-ping example.com")
            # Allow 'ping' to timeout (the currently used busybox ping can't have custom timeout).
            # This takes longer time for incoming traffic because the busybox ping timeout is longer
            # when the host is reachable.
            # compared to when not able to reach host.
            time.sleep(20)
            is_pingable = helper.ping_result()
            assert is_pingable is False
            helper.remove_file()
        finally:
            sc.terminate()


@pytest.mark.usefixtures("testhelper", "agent")
class TestNetworkDiversity(object):
    """ This suite tests that whether NetworkGateway can assign different ip adresses or not

        TODO: This suite is broken at the moment and needs to be adapted to use the
              helper from the test framework.
    """
    def test_ip_diversity(self):
        try:
            sc = Container()
            sc2 = Container()

            sc.start(DATA)
            sc2.start(DATA)

            sc.set_gateway_config("network", GW_CONFIG_POLICY_ACCEPT)
            sc2.set_gateway_config("network", GW_CONFIG_POLICY_ACCEPT)

            sc.launch_command("python " +
                              sc.get_bind_dir() +
                              "/testhelper.py " +
                              " --test-dir " + sc.get_bind_dir() +
                              " --do-ifconfig f1")
            time.sleep(1)
            sc2.launch_command("python " +
                              sc.get_bind_dir() +
                              "/testhelper.py " +
                              " --test-dir " + sc.get_bind_dir() +
                              " --do-ifconfig f2")
            time.sleep(1)
            helper = NetworkHelper(CURRENT_DIR)
            assert 0 == helper.compare_ips("f1", "f2")
            helper.remove_files("f1", "f2")
            #testhelper.remove_file(CURRENT_DIR, "f2")
        finally:
            sc.terminate()
