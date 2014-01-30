#!/usr/bin/env python

import commands, os, time, sys, signal
from subprocess import Popen

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

# Get the PAM-stub
pam_remote_object = bus.get_object("com.pelagicore.PAM", "/com/pelagicore/PAM")
pam_iface = dbus.Interface(pam_remote_object, "com.pelagicore.PAM")

# ----------------------- Helper functions

def cleanup():
    print "Clean up and exit!"
    sys.exit()

def find_app_on_dbus():
    try:
        bus.get_object("com.pelagicore.test.ContainedApp",
            "/com/pelagicore/test/ContainedApp")
        return True
    except:
        return False

# ----------------------- Test functions

def test_can_start_pelagicontain(command):
    global pelagicontain_pid
    try:
        # The intention is to pass a cookie to Pelagicontain which it will use to
        # destinguish itself on D-Bus (as we will potentially have multiple instances
        # running in the system
        pelagicontain_pid = Popen([pelagicontain_binary, "/tmp/test/", command, cookie]).pid
        print "### Will start pelagicontain with the command: " + command
        #pelagicontain_pid = Popen([pelagicontain_binary, "/tmp/test/", command]).pid
    except:
        print "FAIL: Could not start pelagicontain (%s)" % pelagicontain_binary
        cleanup()
    print "PASS: Started Pelagicontain with PID = %s" % pelagicontain_pid

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
        print "PASS: Found Pelagicontain on D-Bus"
    else:
        print "FAIL: Could not find Pelagicontain on D-Bus"
        cleanup()

def test_cant_find_app_on_dbus():
    if not find_app_on_dbus():
        print "PASS: App not present on D-Bus"
    else:
        print "FAIL: App already present on D-Bus"
        cleanup()

def test_can_find_and_run_Launch_in_pelagicontain_on_dbus():
    global pelagicontain_iface
    pelagicontain_iface = dbus.Interface(pelagicontain_remote_object, 
        "com.pelagicore.Pelagicontain")
    try:
        pelagicontain_iface.Launch(app_uuid)
        print "PASS: Found Launch in Pelagicontain on D-Bus"
    except Exception as e:
        print "Fail: Failed to find Launch in Pelagicontain on D-Bus"
        print e
        cleanup()

def test_can_find_app_on_dbus():
    if find_app_on_dbus():
        print "PASS: App present on D-Bus"
    else:
        print "FAIL: App not present on D-Bus"
        cleanup()

def test_registerclient_was_called():
    iterations = 0
    while not pam_iface.test_register_called():# and iterations < 5):
        time.sleep(1)
        iterations = iterations + 1
    print "PASS: RegisterClient was called!"

def test_updatefinished_was_called():
    iterations = 0
    while not pam_iface.test_updatefinished_called():# and iterations < 5):
        time.sleep(1)
        iterations = iterations + 1
    print "PASS: UpdateFinished was called!"

def test_unregisterclient_was_called():
    iterations = 0
    while not pam_iface.test_unregisterclient_called():# and iterations < 5):
        time.sleep(1)
        iterations = iterations + 1
    print "PASS: UnregisterClient was called!"



# ----------------------------- Shutdown

""" NOTE: This is not doing what is eventually intended, see notes below where
    this function is called.
"""
def shutdown_pelagicontain():
    try:
        pelagicontain_iface.Shutdown()
        print "Shutting down Pelagicontain"
    except Exception as e:
        print "FAIL: Failed to call Shutdown on Pelagicontain (over D-Bus)"
        print e


""" Pelagicontain component tests

    The test requires root privileges or some other user with rights to run
    e.g lxc-execute.

    The test requires a FIFO file to be created in the rootfs of the deployed
    app. For example if the second command to Pelagicontain is "/tmp/test",
    a FIFO file named "in_fifo" should be created in "/tmp/test/rootfs/".
    NOTE: This is not the desired final solution, there should be a better
    way for Pelagicontain and the Controller to communicate.

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
test_can_start_pelagicontain("/deployed_app/controller")

""" Assert the Pelagicontain remote object can be found on the bus
"""
test_pelagicontain_found_on_bus()

""" NOTE: This test is disabled as we currently are not starting a D-Bus service
    inside the container as intended. Should be enabled when that works
"""
#test_cant_find_app_on_dbus()

""" Call Launch on Pelagicontain over D-Bus and assert the call goes well.
    This will trigger a call to PAM::RegisterClient over D-Bus which we will
    assert later.
"""
test_can_find_and_run_Launch_in_pelagicontain_on_dbus()

""" Assert against the PAM-stub that RegisterClient was called by Pelagicontain
"""
test_registerclient_was_called()

""" NOTE: Same as above, there's currently no support to test if an app was
    actually started (should be checked by finding it on D-Bus).
"""
#test_can_find_app_on_dbus()

""" The call by Pelagicontain to PAM::RegisterClient would have triggered
    a call by PAM to Pelagicontain::Update which in turn should result in
    a call from Pelagicontain to PAM::UpdateFinished. Assert that call was
    made by Pelagicontain.
"""
test_updatefinished_was_called()


# --------------- Run tests for shutdown

""" NOTE: Currently this only makes Pelagicontain tell Controller to shut down
    the app inside the container, to actually shut down Controller as well we need
    to write a '3' to the FIFO file we use to communicate with Controller. After that
    only Pelagicontain should remain (lxc-execute should have returned when Controller
    exited), and currently we have to kill Pelagicontain explicitly (see next function
    call below).
"""
shutdown_pelagicontain()

""" The call to Pelagicontain::Shutdown should have triggered a call to
    PAM::UnregisterClient
"""
test_unregisterclient_was_called()



""" NOTE: Possible assertions that should be made when Pelagicontain is more
    complete:

    * Issue pelagicontain.Shutdown()

    * Verify that com.pelagicore.pelagicontain.test_app disappears from bus
    and that PAM has not been requested to unregister

    * Verify that PAM receives PAM.unregister($UUID2)

    * Verify that PELAGICONTAIN_PID is no longer running
"""
