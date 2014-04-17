#!/usr/bin/env python

"""
    Copyright (C) 2014 Pelagicore AB
    All rights reserved.
"""

""" Pelagicontain component tests

    The test requires root privileges or some other user with rights to run
    e.g lxc-execute.

    The test requires a PAM-stub to be running on the system, and on the same
    bus as the tests. The stub is implemented as component-test/pam_stub.py. The
    stub exposes an API that mirrors the real PAM and helper methods that the tests
    uses to assert how Pelagicontain interacted with the PAM-stub.

    The test procedure is as follows:

    (Reset PAM-stub from any previous tests)

    (Startup, preloading - Controller is started, gateways are created)
    * Start Pelagicontain, pass the command to be executed in the container
    * Assert Pelagicontain can be found on D-Bus

    (Launching App - Gateways gets configured and activated, Controller starts App)
    * Assert Pelagicontain::Launch can be called
    * Assert the expected call to PAM::RegisterClient was made
    * Assert the expected call to PAM::UpdateFinished was made

    (Shutdown, teardown - Bring down gateways and Controller)
    * Issue 'shutdown' of Pelagicontain
    * Assert the expected call to PAM::UnregisterClient was made

    NOTE: When we have a proper D-Bus service running as an app inside the container
    we can assert more things during shutdown as well.
"""
import pytest

import conftest
from common import ComponentTestHelper

DBUS_GW_CONFIG = """
[{
    "config-session": [],
    "config-system": [
        {
            "direction": "*",
            "interface": "*",
            "object-path": "/org/bluez/*",
            "method": "*"
        },
        {
            "direction": "*",
            "interface": "org.bluez.Manager",
            "object-path": "/",
            "method": "*"
        }
    ]
}]
"""

NETWORK_GW_CONFIG = """
{
    "internet-access": "true",
    "gateway": "10.0.3.1"
}
"""

class TestPelagicontain():
    helper = ComponentTestHelper()
    configs = {"dbus-proxy": DBUS_GW_CONFIG, "networking": NETWORK_GW_CONFIG}

    def abort(self):
        self.helper.result = 1
        self.helper.cleanup_and_finish()

    def test_pelagicontain(self, pelagicontain_binary, container_path):
        self.helper.pam_iface.test_reset_values()
        self.helper.pam_iface.helper_set_configs(self.configs)

        """ Start Pelagicontain, test is passed if Popen succeeds.
            The command to execute inside the container is passed to the test function.
        """

        success = self.helper.start_pelagicontain2(pelagicontain_binary, container_path,
                                              "/controller/controller", False)
        if not success:
            self.abort()

        """ Assert the Pelagicontain remote object can be found on the bus
        """
        success = self.helper.find_pelagicontain_on_dbus()
        if not success:
            self.abort()

        assert self.helper.result == 0
        """ NOTE: This test is disabled as we currently are not starting a D-Bus service
            inside the container as intended. Should be enabled when that works
        """
        #test_cant_find_app_on_dbus()

        """ Call Launch on Pelagicontain over D-Bus and assert the call goes well.
            This will trigger a call to PAM::RegisterClient over D-Bus which we will
            assert later.
        """
        success = self.helper.find_and_run_Launch_on_pelagicontain_on_dbus()
        if not success:
            self.abort()

        assert self.helper.result == 0
        """ Assert against the PAM-stub that RegisterClient was called by Pelagicontain
        """
        success = self.helper.pam_iface.test_register_called()
        if not success:
            self.abort()

        assert self.helper.result == 0
        """ NOTE: Same as above, there's currently no support to test if an app was
            actually started (should be checked by finding it on D-Bus).
        """
        #test_can_find_app_on_dbus()

        """ The call by Pelagicontain to PAM::RegisterClient would have triggered
            a call by PAM to Pelagicontain::Update which in turn should result in
            a call from Pelagicontain to PAM::UpdateFinished. Assert that call was
            made by Pelagicontain.
        """
        success = self.helper.pam_iface.test_updatefinished_called()
        if not success:
            self.abort()

        assert self.helper.result == 0
        # --------------- Run tests for shutdown

        """ NOTE: Possible assertions that should be made when Pelagicontain is more
            complete:

            * Issue pelagicontain.Shutdown()

            * Verify that com.pelagicore.pelagicontain.test_app disappears from bus
            and that PAM has not been requested to unregister

            * Verify that PAM receives PAM.unregister() with the correct appId

            * Verify that PELAGICONTAIN_PID is no longer running
        """
        success = True
        try:
            self.helper.shutdown_pelagicontain()
        except:
            success = False

        if not success:
            self.abort()

        assert self.helper.result == 0

        """ The call to Pelagicontain::Shutdown should have triggered a call to
            PAM::UnregisterClient
        """
        success = self.helper.pam_iface.test_unregisterclient_called()
        if not success:
            self.abort()

        assert self.helper.result == 0
        self.helper.cleanup_and_finish()
