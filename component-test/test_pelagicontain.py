#!/usr/bin/env python

import commands, os, time, sys
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

pelagicontain_binary = "/home/joakim/code/pelagicore/pelagicontain/build/src/pelagicontain"

pelagicontain_remote_object = None
pam_remote_object = None
pelagicontain_pid = None
pelagicontain_iface = None
cookie_uuid = commands.getoutput("uuidgen").strip()
app_uuid = commands.getoutput("uuidgen").strip()
print "Generated CookieUUID = %s, AppUUID = %s" % (cookie_uuid, app_uuid)

pam_remote_object = bus.get_object("com.pelagicore.PAM", "/com/pelagicore/PAM")
pam_iface = dbus.Interface(pam_remote_object, "com.pelagicore.PAM")

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

# ----------------------- Tests

def test_can_start_pelagicontain():
    global pelagicontain_pid
    try:
        # The intention is to pass a cookie to Pelagicontain which it will use to
        # destinguish itself on D-Bus (as we will potentially have multiple instances
        # running in the system
        #pelagicontain_pid = Popen([pelagicontain_binary, "--cookie=%s" % cookie_uuid]).pid
        
        # Just run 'ls' inside the container for now
        pelagicontain_pid = Popen([pelagicontain_binary, "/tmp/test/", "ls"]).pid
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
                    "/com/pelagicore/Pelagicontain")
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

def test_register_was_called():
    iterations = 0
    while not pam_iface.test_register_called():# and iterations < 5):
        time.sleep(1)
        iterations = iterations + 1
    print "PASS: Register called!"

# ----------------------------- Shutdown

def test_updatefinished_was_called():
    iterations = 0
    while not pam_iface.test_updatefinished_called():# and iterations < 5):
        time.sleep(1)
        iterations = iterations + 1
    print "PASS: Register called!"


# --------------- Reset PAM mock
pam_iface.test_reset_values()

# --------------- Run tests for startup
test_can_start_pelagicontain()
test_pelagicontain_found_on_bus()
test_cant_find_app_on_dbus()
test_can_find_and_run_Launch_in_pelagicontain_on_dbus()
test_register_was_called()
test_can_find_app_on_dbus()

# --------------- Run tests for shutdown






# Issue pelagicontain.Shutdown()

# Verify that com.pelagicore.pelagicontain.test_app disappears from bus
# and that PAM has not been requested to unregister

# Verify that PAM receives PAM.unregister($UUID2)

# Verify that PELAGICONTAIN_PID is no longer running
