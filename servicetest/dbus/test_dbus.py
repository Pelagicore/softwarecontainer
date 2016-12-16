
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
import subprocess
import dbusapp

from testframework import Container


CURRENT_DIR = os.path.dirname(os.path.realpath(__file__))


# This function is used by the 'agent' fixture to know where the log should be stored
def logfile_path():
    return CURRENT_DIR + "/test.log"


# The path to where the test app is located, will be passed in the 'data' dict when
# starting the container. The test app is assumed to be located in the
# same directory as this test.
HOST_PATH = os.path.dirname(os.path.abspath(__file__))

# These default values are used to pass various test specific values and
# configurations to the Container helper. Tests that need to add, remove or
# update entries can simply base their dict on this one for convenience.
DATA = {
    Container.CONFIG: '[{"enableWriteBuffer": false}]',
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

@pytest.mark.usefixtures("dbus_launch", "agent", "assert_no_proxy")
class TestDBus(object):
    """ This suite should do some basic testing of the D-Bus setup with the gateway,
        an app in a container, the proxy, etc. After that, the main point is to
        test the things that can't be tested on dbus-proxy alone. For example pure
        testing of gateway configs is best done just using dbus-proxy, but testing
        what happens when an app in the container uses the bus in various other ways,
        e.g. stress testing and load testing, is better done here.
    """

    def test_query_in(self):
        """ Launch server in container and test if a client can communicate with it from the host system """
        for x in range(0, 10):
            ca = Container()
            try:
                success = ca.start(DATA)
                assert success is True
                ca.set_gateway_config("dbus", WL_CONFIG)
                ca.set_gateway_config("dbus", GW_CONFIG)

                # Launch server inside the container
                ca.launch_command('{}/dbusapp.py server'.format(ca.get_bind_dir()))
                time.sleep(1)

                # Launch client on the host
                client = dbusapp.Client()
                client.run()
                assert client.check_all_good_resp() is True
            finally:
                ca.terminate()

    def test_query_out(self):
        """ Launch client in container and test if it communicates out """
        for x in range(0, 10):
            serv = dbusapp.Server()
            serv.start()
            time.sleep(1)

            ca = Container()
            try:
                success = ca.start(DATA)
                assert success is True
                ca.set_gateway_config("dbus", GW_CONFIG)
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
            serv = dbusapp.Server()
            serv.start()
            success = ca.start(DATA)
            assert success is True

            ca.set_gateway_config("dbus", GW_CONFIG)

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
