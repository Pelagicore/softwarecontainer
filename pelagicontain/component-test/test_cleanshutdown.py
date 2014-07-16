#!/usr/bin/env python


"""
    Copyright (C) 2014 Pelagicore AB
    All rights reserved.
"""

""" Check so that proper cleanup is done when exiting / crashing
    pelagicontain. Directories that should be removed are the following
    if container_path is /tmp/container/:
        - /tmp/container/<container-id>/
        - /tmp/container/late_mounts/<container-id>/
"""

from time import sleep
import pytest
import os
import conftest
from common import ComponentTestHelper

SLEEPY_APP = """
#!/bin/sh
sleep 1
"""

helper = ComponentTestHelper()

class TestCleanup(object):
    """ Assert Pelagicontain does not leave any directories behind
        when shutting down. 

        * Start PC
        * Launch app
        * Shutdown
    """

    def test_pc_start_shutdown(self, pelagicontain_binary, container_path, teardown_fixture):
        container_before = os.listdir(container_path)
        late_mounts_before = os.listdir(container_path + "/late_mounts/")
        helper.pam_iface().test_reset_values()
        self.__create_app(container_path)

        """ Start and shutdown """
        helper.start_pelagicontain(pelagicontain_binary, container_path)
        helper.shutdown_pelagicontain()
        sleep(1)
        
        assert self.__dir_diff(container_before, container_path)
        assert self.__dir_diff(late_mounts_before, container_path + "/late_mounts/")

    def test_pc_start_run_shutdown(self, pelagicontain_binary, container_path, teardown_fixture):
        container_before = os.listdir(container_path)
        late_mounts_before = os.listdir(container_path + "/late_mounts/")
        helper.pam_iface().test_reset_values()
        self.__create_app(container_path)

        """ Make a "normal" run """
        helper.start_pelagicontain(pelagicontain_binary, container_path)
        pc_iface = helper.pelagicontain_iface()
        pc_iface.Launch(helper.app_id())
        sleep(2) # Slightly longer than the app

        assert self.__dir_diff(container_before, container_path)
        assert self.__dir_diff(late_mounts_before, container_path + "/late_mounts/")

    def test_pc_start_run_interrupt(self, pelagicontain_binary, container_path, teardown_fixture):
        container_before = os.listdir(container_path)
        late_mounts_before = os.listdir(container_path + "/late_mounts/")
        helper.pam_iface().test_reset_values()
        self.__create_app(container_path)

        """ Interrupt a "normal" run """
        helper.start_pelagicontain(pelagicontain_binary, container_path)
        pc_iface = helper.pelagicontain_iface()
        pc_iface.Launch(helper.app_id())
        helper.shutdown_pelagicontain()
        sleep(2) # For some reason this has to be larger than 1

        assert self.__dir_diff(container_before, container_path)
        assert self.__dir_diff(late_mounts_before, container_path + "/late_mounts/")

    def test_pc_start_interrupt(self, pelagicontain_binary, container_path, teardown_fixture):
        container_before = os.listdir(container_path)
        late_mounts_before = os.listdir(container_path + "/late_mounts/")
        helper.pam_iface().test_reset_values()
        self.__create_app(container_path)

        """ Interrupt before loading an app """
        helper.start_pelagicontain(pelagicontain_binary, container_path)
        helper.shutdown_pelagicontain()
        sleep(1)

        assert self.__dir_diff(container_before, container_path)
        assert self.__dir_diff(late_mounts_before, container_path + "/late_mounts/")

    def __create_app(self, container_path):
        """ Write the app to pelagicontain
        """
        containedapp_file = container_path + "/com.pelagicore.comptest/bin/containedapp"
        with open(containedapp_file, "w") as f:
            print "Overwriting containedapp..."
            f.write(SLEEPY_APP)
        os.system("chmod 755 " + containedapp_file)

    def __dir_diff(self, contents_before, path):
        """ Makes sure that there is no diff between
            a previous listdir of the path and now
        """

        contents_after = os.listdir(path)
        diff = list(set(contents_after) - set(contents_before))
        return len(diff) == 0



