#!/usr/bin/env python

"""
    Copyright (C) 2014 Pelagicore AB
    All rights reserved.
"""

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

helper = ComponentTestHelper()
configs = {"dbus-proxy": DBUS_GW_CONFIG, "networking": NETWORK_GW_CONFIG}
helper.pam_iface.helper_set_configs(configs)

# ----------------------- Test functions

def test_can_start_pelagicontain(command):
    return helper.start_pelagicontain(command)

def test_pelagicontain_found_on_bus():
    return helper.find_pelagicontain_on_dbus()

def test_can_find_and_run_Launch_on_pelagicontain_on_dbus():
    return helper.find_and_run_Launch_on_pelagicontain_on_dbus()

def test_registerclient_was_called():
    return helper.pam_iface.test_register_called()

def test_updatefinished_was_called():
    return helper.pam_iface.test_updatefinished_called()

def test_unregisterclient_was_called():
    return helper.pam_iface.test_unregisterclient_called()


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


# --------------- Reset PAM stub
helper.pam_iface.test_reset_values()


# --------------- Run tests for startup

""" Start Pelagicontain, test is passed if Popen succeeds.
    The command to execute inside the container is passed to the test function.
"""
if test_can_start_pelagicontain("/controller/controller") == False:
    print "FAIL: Could not start Pelagicontain"
    helper.result = 1
    helper.cleanup_and_finish()
else:
    print "PASS: Started Pelagicontain"

""" Assert the Pelagicontain remote object can be found on the bus
"""
if test_pelagicontain_found_on_bus() == False:
    print "FAIL: Could not find Pelagicontain on D-Bus"
    helper.result = 1
    helper.cleanup_and_finish()
else:
    print "PASS: Found Pelagicontain on D-Bus"



""" NOTE: This test is disabled as we currently are not starting a D-Bus service
    inside the container as intended. Should be enabled when that works
"""
#test_cant_find_app_on_dbus()

""" Call Launch on Pelagicontain over D-Bus and assert the call goes well.
    This will trigger a call to PAM::RegisterClient over D-Bus which we will
    assert later.
"""
if test_can_find_and_run_Launch_on_pelagicontain_on_dbus() == False:
    print "FAIL: Failed to find Launch in Pelagicontain on D-Bus"
    helper.result = 1
    helper.cleanup_and_finish()
else:
    print "PASS: Found Launch in Pelagicontain on D-Bus"


""" Assert against the PAM-stub that RegisterClient was called by Pelagicontain
"""
if test_registerclient_was_called() == False:
    print "FAIL: RegisterClient was not called!"
    helper.result = 1
    helper.cleanup_and_finish()
else:
    print "PASS: RegisterClient was called!"


""" NOTE: Same as above, there's currently no support to test if an app was
    actually started (should be checked by finding it on D-Bus).
"""
#test_can_find_app_on_dbus()

""" The call by Pelagicontain to PAM::RegisterClient would have triggered
    a call by PAM to Pelagicontain::Update which in turn should result in
    a call from Pelagicontain to PAM::UpdateFinished. Assert that call was
    made by Pelagicontain.
"""
if test_updatefinished_was_called() == False:
    print "FAIL: UpdateFinished was not called!"
    helper.result = 1
    helper.cleanup_and_finish()
else:
    print "PASS: UpdateFinished was called!"


# --------------- Run tests for shutdown

helper.shutdown_pelagicontain()

""" The call to Pelagicontain::Shutdown should have triggered a call to
    PAM::UnregisterClient
"""
if test_unregisterclient_was_called() == False:
    print "FAIL: UnregisterClient was not called!"
    helper.result = 1
    helper.cleanup_and_finish()
else:
    print "PASS: UnregisterClient was called!"


helper.cleanup_and_finish()

""" NOTE: Possible assertions that should be made when Pelagicontain is more
    complete:

    * Issue pelagicontain.Shutdown()

    * Verify that com.pelagicore.pelagicontain.test_app disappears from bus
    and that PAM has not been requested to unregister

    * Verify that PAM receives PAM.unregister() with the correct appId

    * Verify that PELAGICONTAIN_PID is no longer running
"""
