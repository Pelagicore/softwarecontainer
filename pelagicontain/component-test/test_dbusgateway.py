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
import pytest
import distutils.spawn
import shutil

import conftest

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
                        "interface": "com.pelagicore.comptest",
                        "object-path": "/",
                        "method": "Echo"
                }
        ]
}

configs = {
    "dbus-proxy": json.dumps(config)
}

helper = ComponentTestHelper()
class TestDBusGateway():

    # Run before any tests
    def do_setup(self, pelagicontain_binary, container_path):
        helper.pam_iface().helper_set_configs(configs)
        if not helper.start_pelagicontain2(pelagicontain_binary,
                                           container_path, "/controller/controller", False):
            print "Failed to launch pelagicontain!"
            sys.exit(1)

        # Locate dbus-send binary and copy it into container
        dbus_send = distutils.spawn.find_executable("dbus-send")
        if not dbus_send:
            print "Could not find 'dbus-send'; is it installed?"
            exit(1)

        dest = container_path + "/" + helper.app_uuid + "/bin/"

        print "Copying %s to %s" % (dbus_send, dest)
        shutil.copy(dbus_send, dest)

    # Run after all tests
    def teardown(self):
        try:
            helper.teardown()
        except:
            pass

    # Actual test
    def test_can_introspect(self, pelagicontain_binary, container_path, teardown_fixture):
        """ Tests Introspect() on com.pelagicore.PAM on D-Bus session bus.
        """
        self.do_setup(pelagicontain_binary, container_path)
        with open("%s/com.pelagicore.comptest/bin/containedapp" % container_path, "w") as f:
            print "Overwriting containedapp..."
            f.write("""#!/bin/sh
sleep 1
/appbin/dbus-send --session --print-reply --dest=com.pelagicore.PAM /com/pelagicore/PAM org.freedesktop.DBus.Introspectable.Introspect > /appshared/dbus_test_output""")
        os.system("chmod 755 %s/com.pelagicore.comptest/bin/containedapp" % container_path)

        assert helper.find_pelagicontain_on_dbus()
        assert helper.find_and_run_Launch_on_pelagicontain_on_dbus()

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

        self.teardown()

