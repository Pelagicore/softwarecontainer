#!/usr/bin/env python

"""
    Copyright (C) 2014 Pelagicore AB
    All rights reserved.
"""

""" SoftwareContainer component tests

    The test requires root privileges or some other user with rights to run
    e.g lxc-execute.

    The test requires a PAM-stub to be running on the system, and on the same
    bus as the tests. The stub is implemented as component-test/pam_stub.py. The
    stub exposes an API that mirrors the real PAM and helper methods that the tests
    uses to assert how SoftwareContainer interacted with the PAM-stub.
"""
import os
import time
import pytest

# conftest contains some fixtures we use in the tests
import conftest

from common import ComponentTestHelper


# The app does nothing for so long we have time to call SoftwareContainer::Shutdown ourselves
SLEEPY_APP = """
#!/bin/sh
sleep 10
"""

# We keep the helper object module global so external fixtures can access it
helper = ComponentTestHelper()


class TestSoftwareContainer():

    def test_softwarecontainer(
            self,
            softwarecontainer_binary,
            container_path,
            teardown_fixture):
        """ The test procedure is as follows:

            (Reset PAM-stub from any previous tests)

            (Startup, preloading - Controller is started, gateways are created)
            * Start SoftwareContainer, pass the command to be executed in the container
            * Assert SoftwareContainer can be found on D-Bus

            (Launching App - Gateways gets configured and activated, Controller starts App)
            * Assert SoftwareContainer::Launch can be called
            * Assert the expected call to PAM::RegisterClient was made
            * Assert the expected call to PAM::UpdateFinished was made

            (Shutdown, teardown - Bring down gateways and Controller)
            * Issue 'shutdown' of SoftwareContainer
            * Assert the expected call to PAM::UnregisterClient was made
        """
        helper.pam_iface().test_reset_values()

        # Start SoftwareContainer, test is passed if Popen succeeds.
        # The command to execute inside the container is passed to the test function.
        assert helper.start_softwarecontainer(softwarecontainer_binary, container_path)

        # Assert the SoftwareContainer remote object can be found on the bus
        assert helper.is_service_available()

        # When we run SoftwareContainer::Launch the Controller expects to find an app
        self.create_app(container_path)

        # Call Launch on SoftwareContainer over D-Bus
        helper.softwarecontainer_iface().Launch(helper.app_id())
        time.sleep(0.5)

        # Assert against the PAM-stub that RegisterClient was called by SoftwareContainer
        assert helper.pam_iface().test_register_called()
        time.sleep(0.5)

        # The call by SoftwareContainer to PAM::RegisterClient would have triggered
        # a call by PAM to SoftwareContainer::Update which in turn should result in
        # a call from SoftwareContainer to PAM::UpdateFinished. Assert that call was
        # made by SoftwareContainer.
        assert helper.pam_iface().test_updatefinished_called()

        helper.softwarecontainer_iface().Shutdown()
        # Allow some time for SoftwareContainer to make the subsequent call to PAM
        time.sleep(0.5)

        # The call to SoftwareContainer::Shutdown should have triggered a call to
        # PAM::UnregisterClient
        assert helper.pam_iface().test_unregisterclient_called()

    def test_pc_shuts_down_when_no_containedapp(
            self,
            softwarecontainer_binary,
            container_path,
            teardown_fixture):
        """ Test that PC shuts down if there is no app to laucnh.

            This test assumes all other prerequisites (other than there's no
            file named 'containedapp' where it should be) for running an app
            are fulfilled.

            * Start PC
            * Remove containedapp if there is one (might be left from some test)
            * Launch app (this is a correct app id and so on)
            * Assert PC shuts down as a result

            TODO: This test will pass if PC crashes or exits for some other
            reason as well...
        """
        helper.start_softwarecontainer(softwarecontainer_binary, container_path)
        pc_iface = helper.softwarecontainer_iface()
        self.remove_containedapp_if_present(container_path)
        pc_iface.Launch(helper.app_id())
        time.sleep(1)
        assert not self.softwarecontainer_is_running()

    def test_pc_shuts_down_when_missing_latemount_dir(
            self,
            softwarecontainer_binary,
            teardown_fixture):
        """ Test that PC shuts down if there are crucial container directories
            missing, e.g. '<container-root>/late_mounts/<container-name>'

            * Start SoftwareContainer with a container root that doesn't contain a
              late_mounts directory
            * Assert SoftwareContainer shuts down

            TODO: This test will pass if PC crashes or exits for some other
            reason as well...
        """
        container_path = "/tmp/nonexistent/"
        if not os.path.isdir(container_path):
            os.makedirs(container_path)
        helper.start_softwarecontainer(softwarecontainer_binary, container_path)
        time.sleep(1)
        assert not self.softwarecontainer_is_running()

    def create_app(self, container_path):
        containedapp_file = container_path + "/com.pelagicore.comptest/bin/containedapp"
        with open(containedapp_file, "w") as f:
            print "Overwriting containedapp..."
            f.write(SLEEPY_APP)
        os.system("chmod 755 " + containedapp_file)

    def remove_containedapp_if_present(self, container_path):
        containedapp = container_path + helper.app_id() + "/bin/containedapp"
        if os.path.isfile(containedapp):
            os.remove(containedapp)

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
