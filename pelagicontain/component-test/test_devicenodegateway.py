#!/usr/bin/env python

"""
    Copyright (C) 2014 Pelagicore AB
    All rights reserved.
"""
import json
import sys
import time
import re

from common import ComponentTestHelper

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
                                       "/deployed_app/controller"):
        print "Failed to launch pelagicontain!"
        sys.exit(1)


# Run after all tests
def teardown():
    helper.shutdown_pelagicontain()

# Begin testing sequence


setup()

with open("%s/rootfs/containedapp" % container_root, "w") as f:
    print "Overwriting containedapp..."
    f.write("""#!/bin/sh
    ls -ls /dev/random > /deployed_app/devicenode_test_output""")

time.sleep(2)

helper.pam_iface.helper_trigger_update(helper.cookie, configs)

time.sleep(2)

with open("%s/rootfs/devicenode_test_output" % container_root) as f:
    line = f.readline()
    print "-->", line
    regex = "\s*\d\scrw-rw-rw-\s*\d\s*root\s*root\s*\d,\s*\d.*/dev/random$"
    if re.match(regex, line):
        print "Looks like device was created OK!"

teardown()
