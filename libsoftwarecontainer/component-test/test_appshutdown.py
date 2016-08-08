#!/usr/bin/env python

"""
    Copyright (C) 2014 Pelagicore AB
    All rights reserved.
"""

from time import sleep
import os
import sys

from common import ComponentTestHelper


SLEEPY_APP = """
#!/bin/sh
sleep 1
"""

helper = ComponentTestHelper()


class TestAppShutdown():
    """ Test that SoftwareContainer shuts down when the contained app does.

        Starts SoftwareContainer with an app that sleeps and then exits. Asserts
        that the SoftwareContainer process is not running in the system anymore after
        the app has exited.
    """

    def test_softwarecontainer_shuts_down_when_app_does(
            self,
            softwarecontainer_binary,
            container_path,
            teardown_fixture):
        """ * Start SoftwareContainer
            * Launch the app
            * Make sure the app has time to exit by itself
            * Assert SoftwareContainer is not running after the app has exited
        """
        self.create_app(container_path)
        helper.start_softwarecontainer(softwarecontainer_binary, container_path)
        helper.softwarecontainer_iface().Launch(helper.app_id())
        # Sleep a little bit longer than the app
        sleep(4)
        assert not self.softwarecontainer_is_running()

    def create_app(self, container_path):
        containedapp_file = container_path + "/com.pelagicore.comptest/bin/containedapp"
        with open(containedapp_file, "w") as f:
            print "Overwriting containedapp..."
            f.write(SLEEPY_APP)
        os.system("chmod 755 " + containedapp_file)

    def softwarecontainer_is_running(self):
        """ Return False is SoftwareContainer is not running, True if it is.
        """
        # If the process is a zombie it will be killed by this call, otherwise
        # it's unaffected
        helper.poke_softwarecontainer_zombie()
        pc_pid = helper.softwarecontainer_pid()
        try:
            os.kill(pc_pid, 0)
        except OSError:
            return False
        else:
            return True
