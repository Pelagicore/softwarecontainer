
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

import pytest

import os

from testframework import Container


CURRENT_DIR = os.path.dirname(os.path.realpath(__file__))

##### Configs #####

# These configurations are passed to the Helper to write Service Manifests
# to file which can be read by ConfigStore when initiating the Agent.


# This function is used by the 'agent' fixture to know where the log should be stored
def logfile_path():
    return CURRENT_DIR + "/test.log"

# This function is used by the 'agent' fixture to know where to search for capabilities
def caps_dirs():
    return "{}/caps.d/".format(CURRENT_DIR), "{}/caps.default.d/".format(CURRENT_DIR)

# These default values are used to pass various test specific values and
# configurations to the Container helper. Tests that need to add, remove or
# update entries can simply base their dict on this one for convenience.
DATA = {
    Container.CONFIG: '[{"enableWriteBuffer": false}]',
    Container.BIND_MOUNT_DIR: "/gateways/app",
    Container.HOST_PATH: CURRENT_DIR,
    Container.READONLY: False
}

@pytest.mark.usefixtures("dbus_launch", "agent", "assert_no_proxy")
class TestCaps(object):
    """ This suite tests that capabilities can be used with SoftwareContainer with
        expected results.

        The tests use the Agent D-Bus interface to enable gateway configurations,
        for a specified list of capabilities. Default gateway configurations will
        always be applied.

        The purpose is not to test the gatweays or their configs, which is done in
        more specific test suites, but rather to make sure the capabilities result
        in the correct gateway configs being applied.
    """

    def test_set_caps_only_default(self):
        """ Test that setting the default capabilities works when sending
            an empty list of capabilities, i.e. no error is returned
        """
        sc = Container()
        try:
            success = sc.start(DATA)
            assert success is True

            caps_set = sc.set_capabilities([])
            assert caps_set is True

            ## TODO verify that config is valid and can be used
        finally:
            sc.terminate()

    def test_set_caps(self):
        """ Test that setting an existing capability works,
            i.e. no error is returned
        """
        sc = Container()
        try:
            success = sc.start(DATA)
            assert success is True

            caps, success = sc.list_capabilities()
            assert success is True
            assert "com.pelagicore.sample.simple" in caps

            caps_set = sc.set_capabilities(["com.pelagicore.sample.simple"])
            assert caps_set is True

            ## TODO verify that config is valid and can be used
        finally:
            sc.terminate()

    @pytest.mark.xfail() # See reported issue
    def test_evil_caps(self):
        """ Test that setting a non-existing capability works,
            i.e. no error is returned by the D-Bus API.
            Note: Since we do have a default Service Manifest this test
            will still send configuration to D-BUS.
        """
        sc = Container()
        try:
            success = sc.start(DATA)
            assert success is True

            caps_set = sc.set_capabilities(["test.dbus", "test.network"])
            assert caps_set is False ## This should fail when error handling is revised
        finally:
            sc.terminate()
