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

import time
import os
import json
import pytest

import conftest
from common import ComponentTestHelper

CONFIG_NETWORK_ENABLED = {"internet-access": "true", "gateway": "10.0.3.1"}
CONFIG_NETWORK_DISABLED = {"internet-access": "false", "gateway": ""}
CONFIG_NETWORK_NO_GATEWAY = {"internet-access": "true", "gateway": ""}

TEST_CONFIGS= [CONFIG_NETWORK_ENABLED,
               CONFIG_NETWORK_DISABLED,
               CONFIG_NETWORK_NO_GATEWAY]

helper = ComponentTestHelper()

class TestNetworkGateway():
    """ Tests the NetworkGateway using three different configurations:

        * Well-formed configuration with internet access enabled.
        * Well-formed configuration with internet access disabled.
        * Malformed configuration where gateway has not been specified.
    """
    @pytest.mark.parametrize("config", TEST_CONFIGS)
    def test_has_internet_access(self, pelagicontain_binary, container_path, teardown_fixture,
                                 config):
        helper.pam_iface().helper_set_configs({"networking": json.dumps(config)})
        assert setup(pelagicontain_binary, container_path)
        time.sleep(2)

        assert teardown()
        ping_success = ping_successful(container_path)

        # Build is parametrized, assert based on what config has been passed to the test case
        if "true" in config["internet-access"]:
            if "10.0.3.1" in config["gateway"]:
                assert ping_success
            else:
                assert not ping_success
        else:
            assert not ping_success
        time.sleep(2)

def ping_successful(container_path):
    success = False

    try:
        with open("%s/com.pelagicore.comptest/shared/ping_log" % container_path) as f:
            lines = f.readlines()
            for line in lines:
                if "0% packet loss" in line:
                    success = True
                    break
    except Exception as e:
        pytest.fail("Unable to read command output, output file couldn't be opened! " + \
                    "Exception: %s" % e)
    return success

def setup(pelagicontain_binary, container_path):
    # --------------- Reset PAM stub
    helper.pam_iface().test_reset_values()

    # --------------- Run tests for startup
    """ Start Pelagicontain, test is passed if Popen succeeds.
        The command to execute inside the container is passed to the test function.
    """
    assert helper.start_pelagicontain2(pelagicontain_binary, container_path,
                                   "controller/controller", False)

    """ Assert the Pelagicontain remote object can be found on the bus
    """
    assert helper.find_pelagicontain_on_dbus()

    with open("%s/com.pelagicore.comptest/bin/containedapp" % container_path, "w") as f:
        print "Overwriting containedapp..."
        f.write("""#!/bin/sh
                   ping -c 1 www.google.com > /appshared/ping_log""")
    os.system("chmod 755 %s/com.pelagicore.comptest/bin/containedapp" % container_path)


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
    return True

def teardown():
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

    """ NOTE: Possible assertions that should be made when Pelagicontain is more
        complete:

        * Issue pelagicontain.Shutdown()

        * Verify that com.pelagicore.pelagicontain.test_app disappears from bus
          and that PAM has not been requested to unregister

        * Verify that PAM receives PAM.unregister() with the correct appId

        * Verify that PELAGICONTAIN_PID is no longer running
    """
    return True
