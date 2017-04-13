
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

from testframework.testhelper import CGroupHelper
from testframework import Container
from testframework import Capability
from testframework import StandardManifest

from dbus.exceptions import DBusException

CURRENT_DIR = os.path.dirname(os.path.realpath(__file__))
TESTOUTPUT_DIR = CURRENT_DIR + "/testoutput"

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

"""##### Configs #####"""

# These default values are used to pass various test specific values and
# configurations to the Container helper. Tests that need to add, remove or
# update entries can simply base their dict on this one for convenience.
DATA = {
    Container.CONFIG: '[{ "writeBufferEnabled": false }]',
    Container.BIND_MOUNT_DIR: "/gateways/app",
    Container.HOST_PATH: CURRENT_DIR,
    Container.READONLY: False
}

CONFIG_TEST_MEMORY_SMALL_THRESHOLD = [
    {"setting": "memory.limit_in_bytes", "value": "1K"}
]

CONFIG_TEST_MEMORY_SHARE = [
    {"setting": "memory.limit_in_bytes", "value": "1G"},
    {"setting": "memory.memsw.limit_in_bytes", "value": "1G"}
]

CONFIG_TEST_MEMORY_WHITELISTING = [
    {"setting": "memory.limit_in_bytes", "value": "2K"},
    {"setting": "memory.limit_in_bytes", "value": "1M"},
    {"setting": "memory.memsw.limit_in_bytes", "value": "10M"},
    {"setting": "memory.memsw.limit_in_bytes", "value": "100K"}
]

TEST_NETCLS_VALUE = "0x10001"
CONFIG_TEST_NETCLS = [
    {"setting": "net_cls.classid", "value": TEST_NETCLS_VALUE}
]

CPU_SHARES_LOW_VALUE = "2"
CONFIG_CPU_SHARES_LOW_THRESHOLD = [
    {"setting": "cpu.shares", "value": CPU_SHARES_LOW_VALUE}
]

CPU_SHARES_WHITELISTING_VALUE = "750"
CONFIG_CPU_SHARES_WHITELISTING = [
    {"setting": "cpu.shares", "value": "250"},
    {"setting": "cpu.shares", "value": CPU_SHARES_WHITELISTING_VALUE}
]

cap_0 = Capability("test.cap.small.threshold",
                        [
                            {"id": "cgroups", "config": CONFIG_TEST_MEMORY_SMALL_THRESHOLD}
                        ])

cap_1 = Capability("test.cap.memory.share",
                        [
                            {"id": "cgroups", "config": CONFIG_TEST_MEMORY_SHARE}
                        ])

cap_2 = Capability("test.cap.memory.whitelist",
                        [
                            {"id": "cgroups", "config": CONFIG_TEST_MEMORY_WHITELISTING}
                        ])

cap_3 = Capability("test.cap.netcls",
                        [
                            {"id": "cgroups", "config": CONFIG_TEST_NETCLS}
                        ])

cap_4 = Capability("test.cap.cpu.shares.threshold",
                   [
                       {"id": "cgroups", "config": CONFIG_CPU_SHARES_LOW_THRESHOLD}
                   ])

cap_5 = Capability("test.cap.cpu.shares.whitelist",
                   [
                       {"id": "cgroups", "config": CONFIG_CPU_SHARES_WHITELISTING}
                   ])

manifest = StandardManifest(TESTOUTPUT_DIR,
                            "cgroup-test-manifest.json",
                            [cap_0, cap_1, cap_2, cap_3, cap_4, cap_5])


def service_manifests():
    """ The agent fixture calls this function when it creates the service manifests
        that should be used with this test module. The agent fixture expects a list
        of StandardManifest and/or DefaultManifest objects.
    """
    return [manifest]


"""##### Test suites #####"""


@pytest.mark.usefixtures("testhelper", "create_testoutput_dir", "agent")
class TestCGroupGateway(object):
    """ This suite tests that whether CGroupsGateway can working with supported options or not.

        Prerequisites :
        CONFIG_MEMCG, CONFIG_MEMCG_SWAP and CONFIG_CGROUP_NET_CLASSID kernel options should have
        been enabled for these tests.  Also cgroup_enable=memory and swapaccount=1 options should be
        enabled by grub.

    """
    def test_memory_cgroup_small_threshold(self):
        """ Test if the memory limiting with cgroup gateway is working as expected
            which behavior is when allocated more memory than limit it should fail
        """
        try:
            sc = Container()
            sc.start(DATA)

            with pytest.raises(DBusException) as err:
               sc.set_capabilities(["test.cap.small.threshold"])
            assert err.value.get_dbus_name() == Container.DBUS_EXCEPTION_FAILED

        finally:
            sc.terminate()

    def test_memory_cgroup_limit(self):
        """ Test if the memory limiting with cgroup gateway is working as expected
            which behavior is when allocated more memory than limit it should fail
        """
        try:
            sc = Container()
            sc.start(DATA)

            sc.set_capabilities(["test.cap.memory.share"])

            memory_limitation = 1024 * 1024 * 1024
            sc.launch_command("python " +
                              sc.get_bind_dir() +
                              "/testhelper.py" +
                              " --test-dir " +
                              sc.get_bind_dir() +
                              " --do-allocate " +
                              str(memory_limitation))

            # wait 5 seconds for previous operation to end
            time.sleep(3)

            helper = CGroupHelper(CURRENT_DIR)
            allocation_return = helper.result()
            assert allocation_return < memory_limitation
            helper.remove_file()
        finally:
            sc.terminate()

    def test_memory_cgroup_whitelisting(self):
        """ Test if the whitelisting on cgroup memory is working as expected which behavior
            is more permissive configuration should be applied on memory.limit_in_bytes
        """
        try:
            sc = Container()
            cid = sc.start(DATA)
            containerID = "SC-" + str(cid)
            sc.set_capabilities(["test.cap.memory.whitelist"])
            most_permissive_value = 1024 * 1024

            time.sleep(0.5)
            with open("/sys/fs/cgroup/memory/lxc/" + containerID + "/memory.limit_in_bytes", "r") as fh:
                limit_in_bytes = int(fh.read())

            assert limit_in_bytes == most_permissive_value
            most_permissive_value = 10 * 1024 * 1024
            with open("/sys/fs/cgroup/memory/lxc/" + containerID + "/memory.memsw.limit_in_bytes", "r") as fh:
                memsw_limit = int(fh.read())

            assert memsw_limit == most_permissive_value
        finally:
            sc.terminate()

    def test_netcls_cgroup_set(self):
        """ Test that the classid on net_cls cgroup gets set. Does not currently test if it is
            picked up anywhere.

            TODO: For future, run some network application inside the container, listen for the
                  traffic on the outside and check that the classid's are being set.
        """

        try:
            sc = Container()
            cid = sc.start(DATA)
            containerID = "SC-" + str(cid)

            sc.set_capabilities(["test.cap.netcls"])

            time.sleep(0.5)
            with open("/sys/fs/cgroup/net_cls/lxc/" + containerID + "/net_cls.classid", "r") as fh:
                value = int(fh.read())
                assert value == int(TEST_NETCLS_VALUE, base=16)
        finally:
            sc.terminate()

    def test_cpu_shares_cgroup_set(self):
        """ Test that cpu.shares gets set. Does not test CPU usage.
        """

        try:
            sc = Container()
            cid = sc.start(DATA)
            containerID = "SC-" + str(cid)

            sc.set_capabilities(["test.cap.cpu.shares.threshold"])

            time.sleep(0.5)
            with open("/sys/fs/cgroup/cpu/lxc/" + containerID + "/cpu.shares", "r") as fh:
                value = int(fh.read())
                assert value == int(CPU_SHARES_LOW_VALUE)
        finally:
            sc.terminate()

    def test_cpu_shares_cgroup_whitelisting(self):
        """ Test that whitelisting works when setting cpu.shares. Does not test CPU usage.
        """

        try:
            sc = Container()
            cid = sc.start(DATA)
            containerID = "SC-" + str(cid)
            sc.set_capabilities(["test.cap.cpu.shares.whitelist"])
            most_permissive_value = int(CPU_SHARES_WHITELISTING_VALUE)

            time.sleep(0.5)
            with open("/sys/fs/cgroup/cpu/lxc/" + containerID + "/cpu.shares", "r") as fh:
                value = int(fh.read())

            assert value == most_permissive_value
        finally:
            sc.terminate()
