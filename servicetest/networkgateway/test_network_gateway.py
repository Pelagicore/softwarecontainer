
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

import testhelper
from testframework import Container


CURRENT_DIR = os.path.dirname(os.path.realpath(__file__))


# This function is used by the 'agent' fixture to know where the log should be stored
def logfile_path():
    return CURRENT_DIR + "/test.log"


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



@pytest.mark.usefixtures("agent")
class TestNetworkRules(object):
    """ This suite tests that whether NetworkGateway can parse given configuration and applies it to be used 
        in SoftwareContainer with expected results.

        timeouts are 
            2 seconds for expected connectivity
            10 for expected output discconnectivity
            15 for expected input connectivity

        these durations will be shorten with -i option when possible    
    """
    def test_network_policy_drop(self):
        """
            Test setting network default target to drop and tests the corresponding behavior
            which is being not possible to ping a hostname
        """
        try:
            sc = Container()
            sc.start(DATA)
            sc.set_gateway_config("network", GW_CONFIG_POLICY_DROP)
            sc.launch_command("python " + 
                              sc.get_bind_dir() + 
                              "/testhelper.py " +
                              sc.get_bind_dir() + 
                              " --ping www.example.com") 
            time.sleep(10)
            is_pingable = testhelper.ping_result(CURRENT_DIR)
            assert is_pingable != "0"
            testhelper.remove_file(CURRENT_DIR, "ping_result")
        finally:
            sc.terminate()

    def test_network_policy_accept(self):
        """
            Test setting network default target to accept and tests the corresponding behavior
            which is being possible to ping hostname
        """
        try:
            sc = Container()
            sc.start(DATA)
            sc.set_gateway_config("network", GW_CONFIG_POLICY_ACCEPT)
            sc.launch_command("python " + 
                              sc.get_bind_dir() + 
                              "/testhelper.py " +
                              sc.get_bind_dir() + 
                              " --ping www.example.com") 

            time.sleep(2)
            is_pingable = testhelper.ping_result(CURRENT_DIR)
            assert is_pingable == "0"
            testhelper.remove_file(CURRENT_DIR, "ping_result")
        finally:    
            sc.terminate()

    def test_network_reject_example(self):
        """
            Test setting network default target to accept and tests the corresponding behavior
            which is being possible to ping hostname
        """
        try:
            sc = Container()
            sc.start(DATA)
            sc.set_gateway_config("network", GW_CONFIG_REJECT_EXAMPLE_COM)
            sc.launch_command("python " + 
                              sc.get_bind_dir() + 
                              "/testhelper.py " +
                              sc.get_bind_dir() + 
                              " --ping www.example.com") 
            time.sleep(10)
            is_pingable = testhelper.ping_result(CURRENT_DIR)
            assert is_pingable != "0"
            testhelper.remove_file(CURRENT_DIR, "ping_result")
            sc.launch_command("python " + 
                              sc.get_bind_dir() + 
                              "/testhelper.py " +
                              sc.get_bind_dir() + 
                              " --ping www.google.com") 
            time.sleep(2)
            is_pingable = testhelper.ping_result(CURRENT_DIR)
            assert is_pingable == "0"
            testhelper.remove_file(CURRENT_DIR, "ping_result")
        finally:
            sc.terminate()

    def test_network_reject_example_input(self):
        """
            Test setting network default target to accept and tests the corresponding behavior
            which is being possible to ping hostname
        """
        try:
            sc = Container()
            sc.start(DATA)
            sc.set_gateway_config("network", GW_CONFIG_REJECT_EXAMPLE_COM_INPUT)
            sc.launch_command("python " + 
                              sc.get_bind_dir() + 
                              "/testhelper.py " +
                              sc.get_bind_dir() + 
                              " --ping google.com") 
            time.sleep(2)
            is_pingable = testhelper.ping_result(CURRENT_DIR)
            assert is_pingable == "0"
            testhelper.remove_file(CURRENT_DIR, "ping_result")
            sc.launch_command("python " + 
                              sc.get_bind_dir() + 
                              "/testhelper.py " +
                              sc.get_bind_dir() + 
                              " --ping example.com") 
            time.sleep(20)
            is_pingable = testhelper.ping_result(CURRENT_DIR)
            assert is_pingable != "0"
            testhelper.remove_file(CURRENT_DIR, "ping_result")
        finally:     
            sc.terminate()

@pytest.mark.skip(reason="ip diversity feauture is not yet implemented")
@pytest.mark.usefixtures("agent")
class TestNetworkDiversity(object):
    """ 
        This suite tests that whether NetworkGateway can assign different ip adresses or not 
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
                              sc.get_bind_dir() + 
                              " --ifconfig f1") 
            sc2.launch_command("python " + 
                              sc.get_bind_dir() + 
                              "/testhelper.py " +
                              sc.get_bind_dir() + 
                              " --ifconfig f2") 
            assert 0 == testhelper.compare_ips(CURRENT_DIR, "f1", "f2")
            testhelper.remove_file(CURRENT_DIR, "f1")
            testhelper.remove_file(CURRENT_DIR, "f2")
        finally:    
            sc.terminate()


    
