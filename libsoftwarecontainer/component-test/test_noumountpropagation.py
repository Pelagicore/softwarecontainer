#!/usr/bin/env python

"""
    Copyright (C) 2014 Pelagicore AB
    All rights reserved.
"""

from time import sleep
import os
import sys

from common import ComponentTestHelper


INFINITELOOP_APP = """
#!/bin/sh
while :
do
    sleep 1
done
"""

helper = list()


class TestNoUmountPropagation():
    """ Test that a second container does not disrupt the first container due
        to the unmount of 'late_mounts' propagating.
    """

    def test_second_container_doesnt_propagate_unmounts(
            self,
            softwarecontainer_binary,
            container_path,
            teardown_fixture):
        """
            * Start a container.

            * Launch an app and assert the containedapp file is found in the
              tree under 'late_mounts'.

            * Start another container.

            * Assert the 'containedapp' file in the first container can still
              be found.
        """
        global helper
        first_container = ComponentTestHelper()
        helper.append(first_container)
        second_container = ComponentTestHelper("com.pelagicore.comptest2")
        helper.append(second_container)
        self.create_app(container_path)

        # Check what's in the late_mounts directory before Launch to compare later
        content_before = os.listdir(container_path + "/late_mounts")

        first_container.start_softwarecontainer(softwarecontainer_binary, container_path)

        # Find the name of the dir that appeared when we called Launch
        content_now = os.listdir(container_path + "/late_mounts")

        # Get the name of the newly created directory
        new_dir = ""
        for item in content_now:
            if item not in content_before:
                new_dir = item
                break
        # This is the path where the app is when made available inside the container
        late_mounted_app_path = container_path + "/late_mounts/" + new_dir + "/bin/containedapp"

        first_container.softwarecontainer_iface().Launch(first_container.app_id())
        sleep(1)
        assert os.path.exists(late_mounted_app_path)

        # The creation of a new container should not affect the late mounted
        # dirs for the first container.
        second_container.start_softwarecontainer(softwarecontainer_binary, container_path)
        sleep(1)
        assert os.path.exists(late_mounted_app_path)

        first_container.teardown()
        second_container.teardown()

    def create_app(self, container_path):
        containedapp_file = container_path + "/com.pelagicore.comptest/bin/containedapp"
        with open(containedapp_file, "w") as f:
            print "Overwriting containedapp..."
            f.write(INFINITELOOP_APP)
        os.system("chmod 755 " + containedapp_file)
