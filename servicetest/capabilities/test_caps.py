
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
from testframework import Capability
from testframework import StandardManifest


CURRENT_DIR = os.path.dirname(os.path.realpath(__file__))
TESTOUTPUT_DIR = CURRENT_DIR + "/testoutput"


# This function is used by the 'agent' fixture to know where the log should be stored
def logfile_path():
    return TESTOUTPUT_DIR + "/test.log"


# These default values are used to pass various test specific values and
# configurations to the Container helper. Tests that need to add, remove or
# update entries can simply base their dict on this one for convenience.
DATA = {
    Container.CONFIG: '[{"enableWriteBuffer": false}]',
    Container.BIND_MOUNT_DIR: "/gateways/app",
    Container.HOST_PATH: CURRENT_DIR,
    Container.READONLY: False
}


DBUS_GW_CONFIG = [{
    "dbus-gateway-config-session": [{
        "direction": "outgoing",
        "interface": "*",
        "object-path": "*",
        "method": "*"
    }],
    "dbus-gateway-config-system": []
}]


""" Cap with the purpose of just providing a valid cap """
# TODO: Using this cap should actually fail when we get the error handling correct,
#       see reported issue
test_cap = Capability("test.cap",
                      [
                          {"id": "test", "config": []}
                      ])

""" A valid dbus GW cap to test e.g. listing of available caps """
test_cap_dbus = Capability("test.cap.valid-dbus",
                            [
                                {"id": "dbus", "config": DBUS_GW_CONFIG}
                            ])

test_cap_broken = Capability("test.cap.broken-gw-config",
                             [
                                 {"id": "dbus", "config": ["malformed"]}
                             ])

manifest = StandardManifest(TESTOUTPUT_DIR,
                            "caps-test-manifest.json",
                            [test_cap, test_cap_dbus, test_cap_broken])


def service_manifests():
    """ The agent fixture calls this function when it creates the service manifests
        that should be used with this test module. The agent fixture expects a list
        of StandardManifest and/or DefaultManifest objects.
    """
    return [manifest]


@pytest.fixture
def create_testoutput_dir(scope="module"):
    """ Create a directory for the generated test files.

        This directory is ignored by git but it's nice to have
        somewhere locally to store test output to support
        troubleshooting etc.
    """
    if not os.path.exists(TESTOUTPUT_DIR):
        os.makedirs(TESTOUTPUT_DIR)


""" TODO:
            * Add tests for default caps
"""

@pytest.mark.usefixtures("dbus_launch", "create_testoutput_dir", "agent", "assert_no_proxy")
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

    def test_set_caps_with_empty_arg_is_allowed(self):
        """ Test that there is no error when passing an empty list of caps, i.e. no caps
        """
        sc = Container()
        try:
            sc.start(DATA)
            caps_set = sc.set_capabilities([])
            assert caps_set is True

        finally:
            sc.terminate()

    def test_set_caps_works(self):
        """ Test that setting an existing capability works, i.e. no error is returned
        """
        sc = Container()
        try:
            sc.start(DATA)
            caps_set = sc.set_capabilities(["test.cap.valid-dbus"])
            assert caps_set is True
        finally:
            sc.terminate()

    def test_non_existent_cap_results_in_failure(self):
        """ Test that setting a non-existing capability results in a failure
        """
        sc = Container()
        try:
            sc.start(DATA)
            caps_set = sc.set_capabilities(["does.not.exist"])
            assert caps_set is False
        finally:
            sc.terminate()

    @pytest.mark.xfail()
    def test_setting_capability_with_unkown_gw_id_fails(self):
        """ Setting a capability that results in a gateway config with an unknown
            gateway ID should fail
        """
        sc = Container()
        try:
            sc.start(DATA)
            caps_set = sc.set_capabilities(["test.cap"])
            assert caps_set is False
        finally:
            sc.terminate()


    def test_setting_capability_with_broken_gw_config_fails(self):
        """ Setting a capability that leads to a broken gateway config should fail
        """
        sc = Container()
        try:
            sc.start(DATA)
            caps_set = sc.set_capabilities(["test.cap.broken-gw-config"])
            assert caps_set is False
        finally:
            sc.terminate()

    def test_list_capabilities(self):
        """ Test listing available caps works as expected
        """
        sc = Container()
        try:
            sc.start(DATA)
            caps = sc.list_capabilities()
            # All caps present in the service manifest(s) set up previously should be listed
            assert "test.cap" in caps and "test.cap.valid-dbus" in caps and "test.cap.broken-gw-config" in caps
        finally:
            sc.terminate()
