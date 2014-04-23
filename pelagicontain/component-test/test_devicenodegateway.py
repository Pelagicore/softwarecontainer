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

import conftest

from common import ComponentTestHelper

helper = ComponentTestHelper()

class TestDeviceNodeGatway():
    config = {"devices": [
        {
            "name": "/dev/random",
            "major": "1",
            "minor": "8",
            "mode": "666"
        }
    ]}

    configs = {
        "devicenode": json.dumps(config)
    }

    container_root = None


    # Run before any tests
    def do_setup(self, pelagicontain_binary, container_path):
        helper.pam_iface().helper_set_configs(self.configs)
        if not helper.start_pelagicontain(pelagicontain_binary,
                container_path, "/controller/controller", False):
            print "Failed to launch pelagicontain!"
            sys.exit(1)


    # Run after all tests
    def teardown(self):
        try:
            helper.shutdown_pelagicontain()
        except:
            pass

    def test_can_create_device_node(self, pelagicontain_binary, container_path, teardown_fixture):
        self.do_setup(pelagicontain_binary, container_path)

        with open("%s/com.pelagicore.comptest/bin/containedapp" % container_path, "w") as f:
            print "Overwriting containedapp..."
            f.write("""#!/bin/sh
            ls -la
            ls -ls /dev/random > /appshared/devicenode_test_output""")
        os.system("chmod 755 %s/com.pelagicore.comptest/bin/containedapp" % container_path)

        time.sleep(5)

        helper.pam_iface().helper_trigger_update(helper.cookie, self.configs)

        time.sleep(5)

        assert helper.find_pelagicontain_on_dbus()
        assert helper.find_and_run_Launch_on_pelagicontain_on_dbus()

        time.sleep(10)

        try:
            with open("%s/com.pelagicore.comptest/shared/devicenode_test_output" % container_path) as f:
                line = f.readline()
                print "-->", line
                regex = "\s*\d\scrw-rw-rw-\s*\d\s*root\s*root\s*\d,\s*\d.*/dev/random$"
                assert(re.match(regex, line))
        except Exception as e:
            pytest.fail("Unable to read command output, output file couldn't be opened! Exception: %s" % e)


        self.teardown()
