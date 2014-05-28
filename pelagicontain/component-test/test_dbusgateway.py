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
            "interface": "org.freedesktop.DBus.Introspectable",
            "object-path": "/",
            "method": "Introspect"
        }
    ]
}

CONFIGS = {
    "dbus-proxy": json.dumps(CONFIG)
}

CONFIG_WITHOUT_PERMISSION = {
    "dbus-proxy-config-session": [],
    "dbus-proxy-config-system": []
}

CONFIGS_WITHOUT_PERMISSION = {
    "dbus-proxy": json.dumps(CONFIG_WITHOUT_PERMISSION)
}

CONFIGS_WITH_ERROR = {
    "dbus-proxy": "{}"
}

# The app introspects PAM on D-Bus using dbus-send and writes the output to file
# which content is used later in the assertion
DBUSSEND_APP = """
#!/bin/sh
/appbin/dbus-send --session --print-reply --dest=com.pelagicore.PAM /com/pelagicore/PAM org.freedesktop.DBus.Introspectable.Introspect > /appshared/dbus_test_output-session
/appbin/dbus-send --system --print-reply --dest=org.freedesktop.DBus / org.freedesktop.DBus.Introspectable.Introspect > /appshared/dbus_test_output-system
"""

# We keep the list of helper modules global so external fixtures can access it
helper = list()


class TestDBusGateway():

    def test_can_introspect(self, pelagicontain_binary, container_path, teardown_fixture):
        """ Tests Introspect() on com.pelagicore.PAM on D-Bus session bus and
            org.freedesktop.DBus.Introspectable system bus.
        """
        self.do_setup(pelagicontain_binary, container_path, CONFIGS)

        try:
            # check session output
            with open("%s/com.pelagicore.comptest/shared/dbus_test_output-session" % container_path) as f:
                xml = f.read()
                regex = '<interface name="com\.pelagicore\.PAM">'
                assert re.search(regex, xml)

            # check system output
            with open("%s/com.pelagicore.comptest/shared/dbus_test_output-system" % container_path) as f:
                xml = f.read()
                regex = '<interface name="org\.freedesktop\.DBus">'
                assert re.search(regex, xml)

        except:
            print sys.exc_info()[0]
            exit(1)

    def test_can_not_introspect(self, pelagicontain_binary, container_path, teardown_fixture):
        """ Tests fail of Introspect() on session and system bus, because of empty configs
        """
        self.do_setup(pelagicontain_binary, container_path, CONFIGS_WITHOUT_PERMISSION)

        try:
            # check session output
            with open("%s/com.pelagicore.comptest/shared/dbus_test_output-session" % container_path) as f:
                xml = f.read()
                regex = '<interface name="com\.pelagicore\.PAM">'
                assert None == re.search(regex, xml)

            # check system output
            with open("%s/com.pelagicore.comptest/shared/dbus_test_output-system" % container_path) as f:
                xml = f.read()
                regex = '<interface name="org\.freedesktop\.DBus">'
                assert None == re.search(regex, xml)

        except:
            print sys.exc_info()[0]
            exit(1)

    def test_can_not_introspect_with_error(self, pelagicontain_binary, container_path, teardown_fixture):
        """ Tests fail of Introspect() on session and system bus, because of invalid configs
        """
        self.do_setup(pelagicontain_binary, container_path, CONFIGS_WITH_ERROR)

        try:
            # check session output
            with open("%s/com.pelagicore.comptest/shared/dbus_test_output-session" % container_path) as f:
                xml = f.read()
                regex = '<interface name="com\.pelagicore\.PAM">'
                assert None == re.search(regex, xml)

            # check system output
            with open("%s/com.pelagicore.comptest/shared/dbus_test_output-system" % container_path) as f:
                xml = f.read()
                regex = '<interface name="org\.freedesktop\.DBus">'
                assert None == re.search(regex, xml)

        except:
            print sys.exc_info()[0]
            exit(1)

    def do_setup(self, pelagicontain_binary, container_path, configs):
        the_helper = ComponentTestHelper()
        helper.append(the_helper)
        the_helper.pam_iface().helper_set_configs(configs)

        if not the_helper.start_pelagicontain(pelagicontain_binary, container_path):
            print "Failed to launch pelagicontain!"
            sys.exit(1)

        # Locate dbus-send binary and copy it into container
        dbus_send = distutils.spawn.find_executable("dbus-send")
        if not dbus_send:
            print "Could not find 'dbus-send'; is it installed?"
            exit(1)

        dest = container_path + "/" + the_helper.app_id() + "/bin/"

        print "Copying %s to %s" % (dbus_send, dest)
        shutil.copy(dbus_send, dest)

        containedapp_file = dest + "/containedapp"
        with open(containedapp_file, "w") as f:
            print "Overwriting containedapp..."
            f.write(DBUSSEND_APP)
        os.system("chmod 755 " + containedapp_file)

        the_helper.pelagicontain_iface().Launch(the_helper.app_id())
        time.sleep(0.5)  # sleep so the dbus interface has time to launch, etc.
