#!/usr/bin/env python

"""
    Copyright (C) 2014 Pelagicore AB
    All rights reserved.
"""


import commands, os, time, sys, signal
from subprocess import Popen, call
import os

# You must initialize the gobject/dbus support for threading
# before doing anything.
import gobject
gobject.threads_init()

from dbus import glib
glib.init_threads()

# Create a session bus.
import dbus
import dbus.service
from dbus.mainloop.glib import DBusGMainLoop
bus = dbus.SessionBus()

# Check if user pointed out a path to Pelagicontain
pelagicontain_binary = ""
if os.environ.has_key("PC_BINARY"):
    pelagicontain_binary = str(os.environ["PC_BINARY"])
    if not pelagicontain_binary.endswith("pelagicontain"):
        if not pelagicontain_binary.endswith("/"):
            pelagicontain_binary += "/"
        pelagicontain_binary += "pelagicontain"
else:
    pelagicontain_binary = "pelagicontain"

# Some globals used throughout the tests
pelagicontain_remote_object = None
pam_remote_object = None
pelagicontain_pid = None
pelagicontain_iface = None
# Only use the last part, hyphens are not allowed in D-Bus object paths
cookie = commands.getoutput("uuidgen").strip().split("-").pop()
app_uuid = commands.getoutput("uuidgen").strip()
print "Generated Cookie = %s, appId = %s" % (cookie, app_uuid)

container_root_dir = "/tmp/test/"

# Get the PAM-stub
pam_remote_object = bus.get_object("com.pelagicore.PAM", "/com/pelagicore/PAM")
pam_iface = dbus.Interface(pam_remote_object, "com.pelagicore.PAM")

# Result flag is set to non zero on failure. Return value is set as exit
# status and is used by e.g. ctest to determine test results.
result = 0

# ----------------------- Helper functions

def cleanup_and_finish():
    global result
    if not pelagicontain_pid == 0:
        call(["kill", "-9", str(pelagicontain_pid)])
    if result == 0:
        print "\n-=< All tests passed >=-\n"
    else:
        print "\n-=< Failure >=-\n"
    exit(result)


# ----------------------- Test functions

def test_can_start_pelagicontain(command):
    global pelagicontain_pid
    try:
        # The intention is to pass a cookie to Pelagicontain which it will use to
        # destinguish itself on D-Bus (as we will potentially have multiple instances
        # running in the system
        pelagicontain_pid = Popen([pelagicontain_binary, container_root_dir,
            command, cookie]).pid
    except:
        return False
    return True

def test_pelagicontain_found_on_bus():
    global pelagicontain_remote_object
    tries = 0
    found = False
    while not found and tries < 2:
        try:
            pelagicontain_remote_object = bus.get_object("com.pelagicore.Pelagicontain",
                    "/com/pelagicore/Pelagicontain/" + cookie)
            found = True
        except:
            pass
        time.sleep(1)
        tries = tries + 1
    if found:
        return True
    else:
        return False

def test_can_find_and_run_Launch_on_pelagicontain_on_dbus():
    global pelagicontain_iface
    pelagicontain_iface = dbus.Interface(pelagicontain_remote_object, 
        "com.pelagicore.Pelagicontain")
    try:
        pelagicontain_iface.Launch(app_uuid)
        return True
    except Exception as e:
        print e
        return False

def test_registerclient_was_called():
    return pam_iface.test_register_called()

def test_updatefinished_was_called():
    return pam_iface.test_updatefinished_called()

def test_unregisterclient_was_called():
    return pam_iface.test_unregisterclient_called()


# ----------------------------- Shutdown

""" Calling Shutdown will raise a D-Bus error since Pelagicontain shuts down
    without sending a reply. D-Bus seems to consider it an error that there is
    no reply even if the method itself is not supposed to return anything. We
    catch the exception and ignore it.
"""
def shutdown_pelagicontain():
    try:
        pelagicontain_iface.Shutdown()
        print "Shutting down Pelagicontain"
    except:
        pass


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
pam_iface.test_reset_values()


# --------------- Run tests for startup

""" Start Pelagicontain, test is passed if Popen succeeds.
    The command to execute inside the container is passed to the test function.
"""
if test_can_start_pelagicontain("/deployed_app/controller") == False:
    print "FAIL: Could not start Pelagicontain"
    result = 1
    cleanup_and_finish()
else:
    print "PASS: Started Pelagicontain"

""" Assert the Pelagicontain remote object can be found on the bus
"""
if test_pelagicontain_found_on_bus() == False:
    print "FAIL: Could not find Pelagicontain on D-Bus"
    result = 1
    cleanup_and_finish()
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
    result = 1
    cleanup_and_finish()
else:
    print "PASS: Found Launch in Pelagicontain on D-Bus"


""" Assert against the PAM-stub that RegisterClient was called by Pelagicontain
"""
if test_registerclient_was_called() == False:
    print "FAIL: RegisterClient was not called!"
    result = 1
    cleanup_and_finish()
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
    result = 1
    cleanup_and_finish()
else:
    print "PASS: UpdateFinished was called!"


# --------------- Run tests for shutdown

shutdown_pelagicontain()

""" The call to Pelagicontain::Shutdown should have triggered a call to
    PAM::UnregisterClient
"""
if test_unregisterclient_was_called() == False:
    print "FAIL: UnregisterClient was not called!"
    result = 1
    cleanup_and_finish()
else:
    print "PASS: UnregisterClient was called!"


cleanup_and_finish()

""" NOTE: Possible assertions that should be made when Pelagicontain is more
    complete:

    * Issue pelagicontain.Shutdown()

    * Verify that com.pelagicore.pelagicontain.test_app disappears from bus
    and that PAM has not been requested to unregister

    * Verify that PAM receives PAM.unregister() with the correct appId

    * Verify that PELAGICONTAIN_PID is no longer running
"""
