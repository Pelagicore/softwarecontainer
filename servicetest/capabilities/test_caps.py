
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


# This function is used by the 'agent' fixture to know where the log should be stored
def logfile_path():
    return CURRENT_DIR + "/test.log"

# This function is used by the 'agent' fixture to know where to search for capabilities
def caps_dirs():
    return "{}/caps.d".format(CURRENT_DIR), "{}/caps.default.d".format(CURRENT_DIR)

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

        The tests use the Agent D-Bus interface to drive the tests. Test apps are used
        inside the container to excercise different gateways. The purpose is not to test
        the gatways or their configs, which is done in more specific test suites, but rather
        to make sure the capabilities result in the correct gatway configs being applied.

        TODO: Above is not currently true, these tests are only for the currently stubbed
              implementation in SC.
        TODO: How to piggy-back on the other tests that already do these things?
    """

    def test_caps(self):
        """ Test setting a capability works, i.e. the API is there  on D-Bus
        """
        sc = Container()
        try:
            success = sc.start(DATA)
            assert success is True

            caps_set = sc.set_capabilities(["test.dbus", "test.network"])
            assert caps_set is True
        finally:
            sc.terminate()
