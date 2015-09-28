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


CONFIG = [{
    "dbus-gateway-config-session": [
        {
            "direction": "*",
            "interface": "*",
            "object-path": "*",
            "method": "*"
        }
    ],
    "dbus-gateway-config-system": [
        {
            "direction": "outgoing",
            "interface": "org.freedesktop.DBus.Introspectable",
            "object-path": "/",
            "method": "Introspect"
        }
    ]
}]

CONFIGS = {
    "dbus": json.dumps(CONFIG)
}

CONFIG_WITHOUT_PERMISSION = [{
    "dbus-gateway-config-session": [],
    "dbus-gateway-config-system": []
}]

CONFIGS_WITHOUT_PERMISSION = {
    "dbus-gateway": json.dumps(CONFIG_WITHOUT_PERMISSION)
}

CONFIGS_WITH_ERROR = {
    "dbus-gateway": "[{}]"
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

    def test_can_introspect(self,
                            pelagicontain_binary,
                            container_path,
                            teardown_fixture):
        """ Tests Introspect() on com.pelagicore.PAM on D-Bus session bus and
            org.freedesktop.DBus.Introspectable system bus.
        """
        session_file = container_path + "/com.pelagicore.comptest/shared/dbus_test_output-session"
        system_file = container_path + "/com.pelagicore.comptest/shared/dbus_test_output-system"

        self.remove_output_files(session_file, system_file)
        self.do_setup(pelagicontain_binary, container_path, CONFIGS)
        self.wait_for_output_files(session_file, system_file)

        try:
            # check session output
            with open(session_file) as f:
                xml = f.read()
                regex = '<interface name="com\.pelagicore\.PAM">'
                assert re.search(regex, xml)

            # check system output
            with open(system_file) as f:
                xml = f.read()
                regex = '<interface name="org\.freedesktop\.DBus">'
                assert re.search(regex, xml)

        except Exception as e:
            print e
            exit(1)

    def test_can_not_introspect(self,
                                pelagicontain_binary,
                                container_path,
                                teardown_fixture):
        """ Tests fail of Introspect() on session and system bus, because of
            empty configs
        """
        session_file = container_path + "/com.pelagicore.comptest/shared/dbus_test_output-session"
        system_file = container_path + "/com.pelagicore.comptest/shared/dbus_test_output-system"

        self.remove_output_files(session_file, system_file)
        self.do_setup(pelagicontain_binary, container_path, CONFIGS_WITHOUT_PERMISSION)
        self.wait_for_output_files(session_file, system_file)

        try:
            # check session output
            with open(session_file) as f:
                xml = f.read()
                regex = '<interface name="com\.pelagicore\.PAM">'
                assert None == re.search(regex, xml)

            # check system output
            with open(system_file) as f:
                xml = f.read()
                regex = '<interface name="org\.freedesktop\.DBus">'
                assert None == re.search(regex, xml)

        except Exception as e:
            print e
            exit(1)

    @pytest.mark.skipif("True")
    def test_can_not_introspect_with_error(self,
                                           pelagicontain_binary,
                                           container_path,
                                           teardown_fixture):
        """ NOTE: This test is skipped because the way we test for a failed
            introspection should not rely on the output files. This worked
            previously when the output files existed from previous tests.
            Now we need some way to let the other tests wait for the output
            to be there and this test to have some other way of testing for
            failure.
        """
        """ Tests fail of Introspect() on session and system bus, because
            of invalid configs
        """
        session_file = container_path + "/com.pelagicore.comptest/shared/dbus_test_output-session"
        system_file = container_path + "/com.pelagicore.comptest/shared/dbus_test_output-system"

        self.remove_output_files(session_file, system_file)
        self.do_setup(pelagicontain_binary, container_path, CONFIGS_WITH_ERROR)
        self.wait_for_output_files(session_file, system_file)

        try:
            # check session output
            with open(session_file) as f:
                xml = f.read()
                regex = '<interface name="com\.pelagicore\.PAM">'
                assert None == re.search(regex, xml)

            # check system output
            with open(system_file) as f:
                xml = f.read()
                regex = '<interface name="org\.freedesktop\.DBus">'
                assert None == re.search(regex, xml)

        except Exception as e:
            print e
            exit(1)

    def remove_output_files(self, session_file, system_file):
        try:
            os.remove(session_file)
            os.remove(system_file)
        except Exception as e:
            print e

    def wait_for_output_files(self, session_file, system_file):
        """ NOTE: This assume the file will eventually show up, if it doesn't
            this will crash on the 'stat' part later.
        """
        # Check if file exist, then stat to see if there's content
        tries = 0
        sleep_time = 0.1
        while not os.path.isfile(session_file) and tries < 100:
            time.sleep(sleep_time)
            tries += 1
        tries = 0
        while not os.stat(session_file).st_size > 0 and tries < 100:
            time.sleep(sleep_time)
            tries += 1
        tries = 0
        while not os.path.isfile(system_file) and tries < 100:
            time.sleep(sleep_time)
            tries += 1
        tries = 0
        while not os.stat(system_file).st_size > 0 and tries < 100:
            time.sleep(sleep_time)
            tries += 1

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
