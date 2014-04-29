#!/usr/bin/env python

"""
    Copyright (C) 2014 Pelagicore AB
    All rights reserved.
"""

import pytest

import json
import sys
import time
import re
import os
import distutils.spawn
import shutil

# conftest contains some fixtures we use in the tests
import conftest

from common import ComponentTestHelper


CONFIG = {
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
            "interface": "com.pelagicore.comptest",
            "object-path": "/",
            "method": "Echo"
        }
    ]
}

CONFIGS = {
    "dbus-proxy": json.dumps(CONFIG)
}

# The app introspects PAM on D-Bus using dbus-send and writes the output to file
# which content is used later in the assertion
DBUSSEND_APP = """
#!/bin/sh
/appbin/dbus-send --session --print-reply --dest=com.pelagicore.PAM /com/pelagicore/PAM org.freedesktop.DBus.Introspectable.Introspect > /appshared/dbus_test_output
"""

# We keep the helper object module global so external fixtures can access it
helper = ComponentTestHelper()


class TestDBusGateway():

    def test_can_introspect(self, pelagicontain_binary, container_path, teardown_fixture):
        """ Tests Introspect() on com.pelagicore.PAM on D-Bus session bus.
        """
        self.do_setup(pelagicontain_binary, container_path)
        containedapp_file = container_path + "/com.pelagicore.comptest/bin/containedapp"
        with open(containedapp_file, "w") as f:
            print "Overwriting containedapp..."
            f.write(DBUSSEND_APP)
        os.system("chmod 755 " + containedapp_file)

        helper.pelagicontain_iface().Launch(helper.app_id())
        time.sleep(0.5)

        try:
            with open("%s/com.pelagicore.comptest/shared/dbus_test_output" % container_path) as f:
                line = f.readline()
                regex = "^method"
                assert re.match(regex, line)
        except:
            print "Output file not found"
            exit(1)

    def do_setup(self, pelagicontain_binary, container_path):
        helper.pam_iface().helper_set_configs(CONFIGS)
        if not helper.start_pelagicontain(pelagicontain_binary, container_path):
            print "Failed to launch pelagicontain!"
            sys.exit(1)

        # Locate dbus-send binary and copy it into container
        dbus_send = distutils.spawn.find_executable("dbus-send")
        if not dbus_send:
            print "Could not find 'dbus-send'; is it installed?"
            exit(1)

        dest = container_path + "/" + helper.app_id() + "/bin/"

        print "Copying %s to %s" % (dbus_send, dest)
        shutil.copy(dbus_send, dest)
