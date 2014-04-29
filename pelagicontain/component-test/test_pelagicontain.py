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
"""
import os
import time
import pytest

# conftest contains some fixtures we use in the tests
import conftest

from common import ComponentTestHelper


# The app does nothing for so long we have time to call Pelagicontain::Shutdown ourselves
SLEEPY_APP = """
#!/bin/sh
sleep 10
"""

# We keep the helper object module global so external fixtures can access it
helper = ComponentTestHelper()


class TestPelagicontain():

    def test_pelagicontain(self, pelagicontain_binary, container_path, teardown_fixture):
        helper.pam_iface().test_reset_values()

        # Start Pelagicontain, test is passed if Popen succeeds.
        # The command to execute inside the container is passed to the test function.
        assert helper.start_pelagicontain(pelagicontain_binary, container_path)

        # Assert the Pelagicontain remote object can be found on the bus
        assert helper.is_service_available()

        # When we run Pelagicontain::Launch the Controller expects to find an app
        self.create_app(container_path)

        # Call Launch on Pelagicontain over D-Bus
        helper.pelagicontain_iface().Launch(helper.app_id())

        # Assert against the PAM-stub that RegisterClient was called by Pelagicontain
        assert helper.pam_iface().test_register_called()

        # The call by Pelagicontain to PAM::RegisterClient would have triggered
        # a call by PAM to Pelagicontain::Update which in turn should result in
        # a call from Pelagicontain to PAM::UpdateFinished. Assert that call was
        # made by Pelagicontain.
        assert helper.pam_iface().test_updatefinished_called()

        helper.pelagicontain_iface().Shutdown()
        # Allow some time for Pelagicontain to make the subsequent call to PAM
        time.sleep(0.5)

        # The call to Pelagicontain::Shutdown should have triggered a call to
        # PAM::UnregisterClient
        assert helper.pam_iface().test_unregisterclient_called()

    def create_app(self, container_path):
        containedapp_file = container_path + "/com.pelagicore.comptest/bin/containedapp"
        with open(containedapp_file, "w") as f:
            print "Overwriting containedapp..."
            f.write(SLEEPY_APP)
        os.system("chmod 755 " + containedapp_file)
