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

# conftest contains some fixtures
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
    "internet-access": "false",
    "gateway": ""
}
"""

helper = None

class TestPelagicontain():
    global helper
    helper = ComponentTestHelper()
    configs = {"dbus-proxy": DBUS_GW_CONFIG, "networking": NETWORK_GW_CONFIG}

    def create_empty_app(self, container_path):
        with open("%s/com.pelagicore.comptest/bin/containedapp" % container_path, "w") as f:
            print "Overwriting containedapp..."
            f.write("""#!/bin/sh
                       exit 0""")
        os.system("chmod 755 %s/com.pelagicore.comptest/bin/containedapp" % container_path)

    def create_env_app(self, container_path):
        with open("%s/com.pelagicore.comptest/bin/containedapp" % container_path, "w") as f:
            print "Overwriting containedapp..."
            f.write("""#!/bin/sh
                       env > /appshared/env_log""")

        os.system("chmod 755 %s/com.pelagicore.comptest/bin/containedapp" % container_path)

    def is_env_set(self, container_path):
        success = False

        try:
            with open("%s/com.pelagicore.comptest/shared/env_log" % container_path) as f:
                lines = f.readlines()
                for line in lines:
                    if "test-var=test-val" in line:
                        success = True
                        break
        except Exception as e:
            pytest.fail("Unable to read command output, output file couldn't be opened! " + \
                        "Exception: %s" % e)
        return success

    def test_pelagicontain(self, pelagicontain_binary, container_path, teardown_fixture):
        helper.pam_iface().test_reset_values()
        helper.pam_iface().helper_set_configs(self.configs)

        """ Start Pelagicontain, test is passed if Popen succeeds.
            The command to execute inside the container is passed to the test function.
        """
        assert helper.start_pelagicontain(pelagicontain_binary, container_path,
            "/controller/controller", False)

        """ Assert the Pelagicontain remote object can be found on the bus
        """
        assert helper.find_pelagicontain_on_dbus()

        # When we run Pelagicontain::Launch the Controller expects to find an app
        self.create_empty_app(container_path)

        """ Call Launch on Pelagicontain over D-Bus and assert the call goes well.
            This will trigger a call to PAM::RegisterClient over D-Bus which we will
            assert later.
        """
        assert helper.find_and_run_Launch_on_pelagicontain_on_dbus()

        """ Assert against the PAM-stub that RegisterClient was called by Pelagicontain
        """
        assert helper.pam_iface().test_register_called()

        """ The call by Pelagicontain to PAM::RegisterClient would have triggered
            a call by PAM to Pelagicontain::Update which in turn should result in
            a call from Pelagicontain to PAM::UpdateFinished. Assert that call was
            made by Pelagicontain.
        """
        assert helper.pam_iface().test_updatefinished_called()

        """ Assert that an environment variable can be set within the container
            by calling SetContainerEnvironmentVariable. Requires Launch to be called
            again.
        """
        # Create app that reads environment variables within the container
        helper.pam_iface().helper_set_container_env(helper.cookie, "test-var", "test-val")
        time.sleep(1)
        self.create_env_app(container_path)
        assert helper.find_and_run_Launch_on_pelagicontain_on_dbus()
        time.sleep(1)
        assert self.is_env_set(container_path)

        # --------------- Run tests for shutdown
        success = True
        try:
            helper.teardown()
        except:
            success = False

        assert success

        """ The call to Pelagicontain::Shutdown should have triggered a call to
            PAM::UnregisterClient
        """
        assert helper.pam_iface().test_unregisterclient_called()
