
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
import subprocess

from os import environ

from testframework import Container
from testframework import Capability
from testframework import StandardManifest

import dbusapp


CURRENT_DIR = os.path.dirname(os.path.realpath(__file__))
TESTOUTPUT_DIR = CURRENT_DIR + "/testoutput"

# This function is used by the test framework to know where test specific files should be stored
def output_dir():
    return TESTOUTPUT_DIR


# This function is used by the 'agent' fixture to know where the log should be stored
def logfile_path():
    return TESTOUTPUT_DIR + "/test.log"


# The path to where the test app is located, will be passed in the 'data' dict when
# starting the container. The test app is assumed to be located in the
# same directory as this test.
HOST_PATH = os.path.dirname(os.path.abspath(__file__))

# These default values are used to pass various test specific values and
# configurations to the Container helper. Tests that need to add, remove or
# update entries can simply base their dict on this one for convenience.
DATA = {
    Container.CONFIG: '[{"writeBufferEnabled": false}]',
    Container.BIND_MOUNT_DIR: "/gateways/app",
    Container.HOST_PATH: HOST_PATH,
    Container.READONLY: False
}

# Simple gateway config when something just needs to be passed.
GW_CONFIG = [{
    "dbus-gateway-config-session": [{
        "direction": "outgoing",
        "interface": "*",
        "object-path": "*",
        "method": "*"
    }],
    "dbus-gateway-config-system": []
}]

WL_CONFIG = [{
    "dbus-gateway-config-session": [{
        "direction": "*",
        "interface": "*",
        "object-path": "*",
        "method": "*"
    }],
    "dbus-gateway-config-system": []
}]

GW_CONFIG_DENY_ALL = [{
    "dbus-gateway-config-session": [{
        "direction": "",
        "interface": "",
        "object-path": "",
        "method": ""
    }],
    "dbus-gateway-config-system": []
}]

GW_CONFIG_ALLOW_PING = [{
    "dbus-gateway-config-session": [{
        "direction": "outgoing",
        "interface": "*",
        "object-path": "*",
        "method": ["Ping", "Introspect"]
    }],
    "dbus-gateway-config-system": []
}]

""" Cap with only GW_CONFIG"""
test_cap_1 = Capability("test.cap.gwconfig",
                        [
                            {"id": "dbus", "config": GW_CONFIG}
                        ])

""" Cap with only WL_CONFIG """
test_cap_2 = Capability("test.cap.wlconfig",
                        [
                            {"id": "dbus", "config": WL_CONFIG}
                        ])

""" Cap with only GW_CONFIG_DENY_ALL """
test_cap_deny_all = Capability("test.cap.denyall",
                               [
                                    {"id": "dbus", "config": GW_CONFIG_DENY_ALL}
                               ])

""" Cap with only GW_CONFIG_ALLOW_PING """
test_cap_allow_ping = Capability("test.cap.allowping",
                                 [
                                     {"id": "dbus", "config": GW_CONFIG_ALLOW_PING}
                                 ])

manifest = StandardManifest(TESTOUTPUT_DIR,
                            "dbus-test-manifest.json",
                            [test_cap_1, test_cap_2, test_cap_deny_all, test_cap_allow_ping])


def service_manifests():
    """ The agent fixture calls this function when it creates the service manifests
        that should be used with this test module. The agent fixture expects a list
        of StandardManifest and/or DefaultManifest objects.
    """
    return [manifest]


@pytest.mark.usefixtures("dbus_launch", "create_testoutput_dir", "agent", "assert_no_proxy")
class TestDBus(object):
    """ This suite should do some basic testing of the D-Bus setup with the gateway,
        an app in a container, the proxy, etc. After that, the main point is to
        test the things that can't be tested on dbus-proxy alone. For example pure
        testing of gateway configs is best done just using dbus-proxy, but testing
        what happens when an app in the container uses the bus in various other ways,
        e.g. stress testing and load testing, is better done here.
    """

    def test_client_inside_reaches_outside_service(self):
        """ Test that an app running inside a container can call allowed methods
            on service outside of the container.
        """
        c = Container()
        service = subprocess.Popen([
                                        "python",
                                        CURRENT_DIR + "/dbusapp.py",
                                        "server",
                                        "--outdir",
                                        TESTOUTPUT_DIR
                                    ],
                                    env=environ.copy(),
                                    stdout=subprocess.PIPE)
        try:
            c.start(DATA)
            c.set_capabilities(["test.cap.gwconfig"])
            c.launch_command('{}/dbusapp.py client --method Ping'.format(c.get_bind_dir()))

            time.sleep(0.5)

            with open(TESTOUTPUT_DIR + "/service_output", "r") as fh:
                content = fh.read()
            assert "Hello" in content
        finally:
            c.terminate()
            service.terminate()

    def test_reconfiguration(self):
        """ Test that a client inside a container can't initially call a method
            on the service, and that it can call it after another more permissive
            cap is set.

            * Set a restrictive cap
            * Assert client can't call method
            * Set a more permissive cap
            * Assert client can call method
        """
        c = Container()
        service = subprocess.Popen([
                                        "python",
                                        CURRENT_DIR + "/dbusapp.py",
                                        "server",
                                        "--outdir",
                                        TESTOUTPUT_DIR
                                    ],
                                    env=environ.copy(),
                                    stdout=subprocess.PIPE)
        try:
            c.start(DATA)
            c.set_capabilities(["test.cap.denyall"])
            c.launch_command('{}/dbusapp.py client --method Ping'.format(c.get_bind_dir()))

            time.sleep(0.5)

            with open(TESTOUTPUT_DIR + "/service_output", "r") as fh:
                content = fh.read()
            assert "Hello" not in content

            c.set_capabilities(["test.cap.allowping"])
            c.launch_command('{}/dbusapp.py client --method Ping'.format(c.get_bind_dir()))

            time.sleep(0.5)

            with open(TESTOUTPUT_DIR + "/service_output", "r") as fh:
                content = fh.read()
            assert "Hello" in content
        finally:
            c.terminate()
            service.terminate()
 
    def test_query_in(self):
        """ Launch server in container and test if a client can communicate with it from the host system """
        for x in range(0, 10):
            ca = Container()
            try:
                ca.start(DATA)
                ca.set_capabilities(["test.cap.gwconfig", "test.cap.wlconfig"])
                ca.launch_command('{}/dbusapp.py server'.format(ca.get_bind_dir()))

                time.sleep(0.5)
                client = dbusapp.Client()
                client.run()
                assert client.check_all_good_resp() is True
            finally:
                ca.terminate()

    @pytest.mark.skip(reason="See reported issue about this")
    def test_query_out(self):
        """ Launch client in container and test if it communicates out """
        for x in range(0, 10):
            serv = dbusapp.Server()
            serv.start()
            ca = Container()
            try:
                ca.start(DATA)
                ca.set_capabilities(["test.cap.gwconfig"])
                ca.launch_command('{}/dbusapp.py client'.format(ca.get_bind_dir()))

                assert serv.wait_until_requests() is True
            finally:
                ca.terminate()
                serv.terminate()
                serv = None

    @pytest.mark.skip(reason="See reported issue about this")
    def test_spam_out(self):
        """ Launch client in container and stress test the communication out """
        ca = Container()
        try:
            serv = dbusapp.Server(TESTOUTPUT_DIR)
            serv.start()
            ca.start(DATA)

            ca.set_capabilities(["test.cap.gwconfig"])

            clients = 100
            message_size = 8192  # Bytes

            t0 = time.time()
            for x in range(0, clients):
                ca.launch_command('{}/dbusapp.py client --size {}'.format(ca.get_bind_dir(), message_size))
            t1 = time.time()
            assert serv.wait_until_requests(multiplier=clients) is True
            t2 = time.time()
            print("\n")
            print("Clients started:              {0:.4f} seconds\n".format(t1 - t0))
            print("Server received all messages: {0:.4f} seconds\n".format(t2 - t1))
            print("Total time:                   {0:.4f} seconds\n".format(t2 - t0))

        finally:
            ca.terminate()
            serv.terminate()
            serv = None
