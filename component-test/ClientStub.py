#!/usr/bin/env python
# You must initialize the gobject/dbus support for threading
# before doing anything.
import commands, os, time, sys
import gobject
from subprocess import Popen
from threading import Thread
gobject.threads_init()

from dbus import glib
glib.init_threads()

# Create a session bus.
import dbus
import dbus.service
from dbus.mainloop.glib import DBusGMainLoop
bus = dbus.SessionBus()

pelagicontain_binary = "pelagicontain"

# start pelagicontain via system (), set unique cookie using --cookie
# good example is a UUID
remote_object     = None
pam_remote_object = None
pelagicontain_pid = None
iface             = None
pam_iface         = None
cookie_uuid = commands.getoutput("uuidgen").strip()
app_uuid = commands.getoutput("uuidgen").strip()
print "Generated CookieUUID = %s, AppUUID = %s" % (cookie_uuid, app_uuid)


pam_remote_object = bus.get_object("com.pelagicore.PAM",
			       "/com/pelagicore/PAM")
pam_iface = dbus.Interface (pam_remote_object, "com.pelagicore.PAM")

def cleanup():
	print "Clean up and exit!"
	sys.exit()

def find_app_on_dbus ():
	try:
		remote_object2 = bus.get_object("com.pelagicore.test.pelagicontaintestapp", #TODO: This should be pelagicontain's app!!
					       "/com/pelagicore/test/pelagicontaintestapp") #TODO: This should be pelagicontain's app!!
		return True
	except:
		return False

# ----------------------- Tests

def test_can_start_pelagicontain_via_Popen ():
	global pelagicontain_pid
	try:
		pelagicontain_pid = Popen([pelagicontain_binary,
		                    "--cookie=%s" % cookie_uuid]).pid
	except:
		print "FAIL: Could not start pelagicontain (%s)" % \
		      pelagicontain_binary
		cleanup()
	print "PASS: Started Pelagicontain with PID = %s" % pelagicontain_pid

def test_pelagicontain_found_on_bus ():
	global remote_object
	tries = 0
	found = False
	while (not found and tries < 1):
		try:
			remote_object = bus.get_object("com.pelagicore.Pelagicontain",
						       "/com/pelagicore/Pelagicontain")
			found = True
		except:
			pass
		#print "Pelagicontain not found on bus"
		time.sleep(1)
		tries = tries + 1
	if found:
		print "PASS: Found Pelagicontain on D-Bus"
	else:
		print "FAIL: Could not find Pelagicontain on D-Bus"
		cleanup()

def test_can_find_and_run_Launch_in_pelagicontain_on_dbus ():
	global iface
	iface = dbus.Interface (remote_object, "com.pelagicore.Pelagicontain")
	try:
		iface.Launch(app_uuid)
		print "PASS: Found Launch in Pelagicontain on D-Bus"
	except Exception as e:
		print "Fail: Failed to find Launch in Pelagicontain on D-Bus"
		print e
		cleanup()


def test_cant_find_app_on_dbus ():
	if not find_app_on_dbus ():
		print "PASS: App not present on D-Bus"
	else:
		print "FAIL: App already present on D-Bus"
		cleanup()

def test_can_find_app_on_dbus ():
	if find_app_on_dbus ():
		print "PASS: App present on D-Bus"
	else:
		print "FAIL: App not present on D-Bus"
		cleanup()

# ----------------------------- Shutdown

def test_register_was_called ():
	print pam_iface
	iterations = 0
	while (not pam_iface.test_register_called() and iterations < 5):
		time.sleep(1)
		iterations = iterations + 1
	print "PASS: Register called!"

def test_updatefinished_was_called ():
	iterations = 0
	while (not pam_iface.test_updatefinished_called()):# and iterations < 5):
		time.sleep(1)
		iterations = iterations + 1
	print "PASS: Register called!"

# --------------- Run tests for startup
pam_iface.test_reset_values ()

test_can_start_pelagicontain_via_Popen ()
test_pelagicontain_found_on_bus ()
test_cant_find_app_on_dbus ()
test_can_find_and_run_Launch_in_pelagicontain_on_dbus ()
test_register_was_called ()
test_can_find_app_on_dbus ()



# --------------- Run tests for shutdown






# Issue pelagicontain.Shutdown()

# Verify that com.pelagicore.pelagicontain.test_app disappears from bus
# and that PAM has not been requested to unregister

# Verify that PAM receives PAM.unregister($UUID2)

# Verify that PELAGICONTAIN_PID is no longer running

# Create an object that will proxy for a particular remote object.
#remote_object = bus.get_object("com.pelagicore.PAM", # Connection name
#                               "/com/pelagicore/PAM" # Object's path
#                              )
#iface = dbus.Interface (remote_object, "com.pelagicore.PAM")





#received = iface.Config("ClientStubApplication", "dummy-gw1")
#test = "Found cookie in PAM response"
#if "cookie123" in received:
#	print "PASS: %s" % test
#else:
#	print "FAIL: %s" % test

