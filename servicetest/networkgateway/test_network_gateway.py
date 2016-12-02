
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
    Container.BIND_MOUNT_DIR: "/gateways/app",
    Container.HOST_PATH: CURRENT_DIR,
    Container.READONLY: False
}

GW_CONFIG_POLICY_DROP = [
    {
        "direction": "OUTGOING",
        "allow": []
    }
]

""" Dns lookup is needed for resolving domain name for ping purposes.
    DNS uses tcp for zone transfer over Port: 53 and uses udp for
    queries over Port: 53. To enable dns lookup mentioned ports and
    protocols are added to allow list
"""
GW_CONFIG_POLICY_ACCEPT_PING = [
    {
        "direction": "OUTGOING",
        "allow": [{"host": "*", "protocols": "icmp"},
                  {"host": "*", "protocols": ["udp", "tcp"], "ports": 53}]
    },
    {
        "direction": "INCOMING",
        "allow": [{"host": "*", "protocols": "icmp"},
                  {"host": "*", "protocols": ["udp", "tcp"], "ports": 53}]
    }
]

GW_CONFIG_ACCEPT_EXAMPLE_COM = [
    {
        "direction": "OUTGOING",
        "allow": [{"host": "example.com", "protocols": "icmp"},
                  {"host": "*", "protocols": ["udp", "tcp"], "ports": 53}]
    },
    {
        "direction": "INCOMING",
        "allow": [{"host": "example.com", "protocols": "icmp"},
                  {"host": "*", "protocols": ["udp", "tcp"], "ports": 53}]
    }
]

GW_CONFIG_ACCEPT_IANA_ORG = [
    {
        "direction": "OUTGOING",
        "allow": [{"host": "iana.org", "protocols": "icmp"}]
    },
    {
        "direction": "INCOMING",
        "allow": [{"host": "iana.org", "protocols": "icmp"}]
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
            15 for expected input diconnectivity

        these durations will be shorten with -i option when possible
    """
    def test_no_network_rules(self):
        """ Test that packets are dropped if there are no network
            rules given
        """
        try:
            sc = Container()
            sc.start(DATA)

            sc.set_gateway_config("network", [])

            sc.launch_command("python " +
                              sc.get_bind_dir() +
                              "/testhelper.py" +
                              " --test-dir " + sc.get_bind_dir() +
                              " --do-ping 8.8.8.8")
            # Allow 'ping' to timeout (the currently used busybox ping can't have custom timeout)
            time.sleep(10)
            helper = NetworkHelper(CURRENT_DIR)
            is_pingable = helper.ping_result()
            assert is_pingable is False
            # TODO: Move cleanup to a fixture (in all tests)
            helper.remove_file()
        finally:
            sc.terminate()

    def test_network_policy_drop(self):
        """ Test if the default policy is DROP as it should be
            which behavior is being not possible to ping a hostname
        """
        try:
            sc = Container()
            sc.start(DATA)

            sc.set_gateway_config("network", GW_CONFIG_POLICY_DROP)

            sc.launch_command("python " +
                              sc.get_bind_dir() +
                              "/testhelper.py" +
                              " --test-dir " + sc.get_bind_dir() +
                              " --do-ping 8.8.8.8")
            # Allow 'ping' to timeout (the currently used busybox ping can't have custom timeout)
            time.sleep(10)
            helper = NetworkHelper(CURRENT_DIR)
            is_pingable = helper.ping_result()
            assert is_pingable is False
            helper.remove_file()
        finally:
            sc.terminate()

    def test_network_protocols(self):
        """ Tests all allowed protocols and port filtering
            icmp protocol is used for ping hostname
            dns is needed for resolving hostname
            tcp and udp protocols are used for dns lookup
            dns uses tcp for zone transfer over port 53 and it
            uses udp for queries over port 53
        """
        try:
            sc = Container()
            sc.start(DATA)

            sc.set_gateway_config("network", GW_CONFIG_POLICY_ACCEPT_PING)

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


    def test_allow_ping_only_for_specific_domain(self):
        """ Tests the specific domain is allowed to ping.
            And other domains are not allowed to ping.
        """
        try:
            sc = Container()
            sc.start(DATA)

            sc.set_gateway_config("network", GW_CONFIG_ACCEPT_EXAMPLE_COM)

            sc.launch_command("python " +
                              sc.get_bind_dir() +
                              "/testhelper.py" +
                              " --test-dir " + sc.get_bind_dir() +
                              " --do-ping IANA.com")
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
                              " --do-ping example.com")
            time.sleep(2)
            is_pingable = helper.ping_result()
            assert is_pingable is True
            helper.remove_file()
        finally:
            sc.terminate()

    def test_whitelist(self):
        """ Tests the whitelisting feature.
            a config for allowing ping to example.com is set first
            then another config for allowing ping to iana.org is set
            consequences are questioned with asserts
        """
        try:
            sc = Container()
            sc.start(DATA)

            sc.set_gateway_config("network", GW_CONFIG_ACCEPT_EXAMPLE_COM)
            sc.set_gateway_config("network", GW_CONFIG_ACCEPT_IANA_ORG)
            sc.launch_command("python " +
                              sc.get_bind_dir() +
                              "/testhelper.py" +
                              " --test-dir " + sc.get_bind_dir() +
                              " --do-ping iana.org")
            time.sleep(2)
            helper = NetworkHelper(CURRENT_DIR)
            is_pingable = helper.ping_result()
            assert is_pingable is True
            helper.remove_file()

            sc.launch_command("python " +
                              sc.get_bind_dir() +
                              "/testhelper.py" +
                              " --test-dir " + sc.get_bind_dir() +
                              " --do-ping example.org")
            time.sleep(2)
            helper = NetworkHelper(CURRENT_DIR)
            is_pingable = helper.ping_result()
            assert is_pingable is True
            helper.remove_file()

            sc.launch_command("python " +
                              sc.get_bind_dir() +
                              "/testhelper.py" +
                              " --test-dir " + sc.get_bind_dir() +
                              " --do-ping pelagicore.com")
            time.sleep(10)
            helper = NetworkHelper(CURRENT_DIR)
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

            sc.set_gateway_config("network", GW_CONFIG_POLICY_ACCEPT_PING)

            sc2.set_gateway_config("network", GW_CONFIG_POLICY_ACCEPT_PING)

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
