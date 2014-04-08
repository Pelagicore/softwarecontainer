#!/usr/bin/env python

"""
    Copyright (C) 2014 Pelagicore AB
    All rights reserved.
"""
import json
import sys
import time
import re
import os

from common import ComponentTestHelper

config = {
	"dbus-proxy-config-session": [
		{
			"direction": "*",
			"interface": "*",
			"object-path": "*",
			"method": "*"
		}
	],
	"dbus-proxy-config-system": [
		{
			"direction": "outgoing",
			"interface": "com.pelagicore.copmtest",
			"object-path": "/",
			"method": "Echo"
		}
	]
}

configs = {
    "dbus-proxy": json.dumps(config)
}

helper = ComponentTestHelper()
container_root = None


# Check that sys.argv is usable, notify user otherwise. Exits on failure
def ensure_command_line_ok():
    if len(sys.argv) != 3:
        print "Proper invocation looks like this:"
        print "%s <pelagicontain path> <container path>" % sys.argv[0]
        sys.exit(1)


# Run before any tests
def setup():
    global container_root
    ensure_command_line_ok()
    pelagicontain_binary = sys.argv[1]
    container_root = sys.argv[2]

    helper.pam_iface.helper_set_configs(configs)
    if not helper.start_pelagicontain2(pelagicontain_binary, container_root,
                                       "/controller/controller"):
        print "Failed to launch pelagicontain!"
        sys.exit(1)

    os.system("cp `which dbus-send` " + container_root + "/" + helper.app_uuid + "/bin/")


# Run after all tests
def teardown():
    helper.shutdown_pelagicontain()

# Begin testing sequence


setup()

with open("%s/com.pelagicore.comptest/bin/containedapp" % container_root, "w") as f:
    print "Overwriting containedapp..."
    f.write("""#!/bin/sh
    sleep 1
    /appbin/dbus-send --session --print-reply --dest=com.pelagicore.PAM /com/pelagicore/PAM org.freedesktop.DBus.Introspectable.Introspect > /appshared/dbus_test_output""")
os.system("chmod 755 %s/com.pelagicore.comptest/bin/containedapp" % container_root)

helper.find_pelagicontain_on_dbus()
helper.find_and_run_Launch_on_pelagicontain_on_dbus()

time.sleep(2)

try:
    with open("%s/com.pelagicore.comptest/shared/dbus_test_output" % container_root) as f:
        line = f.readline()
        print "-->", line
        regex = "^method"
        if re.match(regex, line):
            print "Looks like dbus-send was run OK!"
except:
    print "Output file not found"

teardown()
