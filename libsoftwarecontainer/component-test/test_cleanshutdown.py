#!/usr/bin/env python

# Copyright (C) 2016 Pelagicore AB
#
# Permission to use, copy, modify, and/or distribute this software for
# any purpose with or without fee is hereby granted, provided that the
# above copyright notice and this permission notice appear in all copies.
#
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
# WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR
# BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES
# OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
# WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
# ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
# SOFTWARE.
#
# For further information see LICENSE


""" Check so that proper cleanup is done when exiting / crashing
    softwarecontainer. Directories that should be removed are the following
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
    """ Assert SoftwareContainer does not leave any directories behind
        when shutting down. 

        * Start PC
        * Launch app
        * Shutdown
    """

    def test_pc_start_shutdown(self, softwarecontainer_binary, container_path, teardown_fixture):
        container_before = os.listdir(container_path)
        late_mounts_before = os.listdir(container_path + "/late_mounts/")
        helper.pam_iface().test_reset_values()
        self.__create_app(container_path)

        """ Start and shutdown """
        helper.start_softwarecontainer(softwarecontainer_binary, container_path)
        helper.shutdown_softwarecontainer()
        sleep(1)
        
        assert self.__dir_diff(container_before, container_path)
        assert self.__dir_diff(late_mounts_before, container_path + "/late_mounts/")

    def test_pc_start_run_shutdown(self, softwarecontainer_binary, container_path, teardown_fixture):
        container_before = os.listdir(container_path)
        late_mounts_before = os.listdir(container_path + "/late_mounts/")
        helper.pam_iface().test_reset_values()
        self.__create_app(container_path)

        """ Make a "normal" run """
        helper.start_softwarecontainer(softwarecontainer_binary, container_path)
        pc_iface = helper.softwarecontainer_iface()
        pc_iface.Launch(helper.app_id())
        sleep(2) # Slightly longer than the app

        assert self.__dir_diff(container_before, container_path)
        assert self.__dir_diff(late_mounts_before, container_path + "/late_mounts/")

    def test_pc_start_run_interrupt(self, softwarecontainer_binary, container_path, teardown_fixture):
        container_before = os.listdir(container_path)
        late_mounts_before = os.listdir(container_path + "/late_mounts/")
        helper.pam_iface().test_reset_values()
        self.__create_app(container_path)

        """ Interrupt a "normal" run """
        helper.start_softwarecontainer(softwarecontainer_binary, container_path)
        pc_iface = helper.softwarecontainer_iface()
        pc_iface.Launch(helper.app_id())
        helper.shutdown_softwarecontainer()
        sleep(2) # For some reason this has to be larger than 1

        assert self.__dir_diff(container_before, container_path)
        assert self.__dir_diff(late_mounts_before, container_path + "/late_mounts/")

    def __create_app(self, container_path):
        """ Write the app to softwarecontainer
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



