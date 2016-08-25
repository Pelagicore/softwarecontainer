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


import time
import os
import json
import pytest

# conftest contains some fixtures we use in the tests
import conftest

from common import ComponentTestHelper


CONFIG_NETWORK_ENABLED = [{"internet-access": True, "gateway": "10.0.3.1"}]
CONFIG_NETWORK_DISABLED = [{"internet-access": False, "gateway": ""}]
CONFIG_NETWORK_NO_GATEWAY = [{"internet-access": True , "gateway": ""}]

TEST_CONFIGS = [CONFIG_NETWORK_DISABLED,
                CONFIG_NETWORK_NO_GATEWAY,
                CONFIG_NETWORK_ENABLED]

PING_APP = """
#!/bin/sh
ping -c 1 www.google.com > /appshared/ping_log
"""

# We keep the helper object module global so external fixtures can access it
helper = ComponentTestHelper()


class TestNetworkGateway():
    """ Tests the NetworkGateway using three different configurations:

        * Well-formed configuration with internet access disabled.
        * Malformed configuration where gateway has not been specified.
        * Well-formed configuration with internet access enabled.
    """
    def test_has_internet_access(self, setup, container_path):
        config = setup
        ping_success = ping_successful(container_path)

        # Build is parametrized, assert based on what config has been passed
        # to the test case
        if config[0]["internet-access"]:
            if "10.0.3.1" in config[0]["gateway"]:
                assert ping_success
            else:
                assert not ping_success
        else:
            assert not ping_success

        helper.teardown()


def ping_successful(container_path):
    success = False

    try:
        with open("%s/com.pelagicore.comptest/shared/ping_log" % container_path) as f:
            lines = f.readlines()
            for line in lines:
                if "0% packet loss" in line:
                    success = True
                    break
    except Exception as e:
        pytest.fail("Unable to read command output, output file couldn't be opened! "
                    "Exception: %s" % e)
    return success


@pytest.fixture(params=TEST_CONFIGS)
def setup(request, teardown_fixture, softwarecontainer_binary, container_path):
    helper.pam_iface().test_reset_values()
    helper.pam_iface().helper_set_configs({"network": json.dumps(request.param)})

    assert helper.start_softwarecontainer(softwarecontainer_binary, container_path)

    # Create the app
    containedapp_file = container_path + "/com.pelagicore.comptest/bin/containedapp"
    with open(containedapp_file, "w") as f:
        print "Overwriting containedapp..."
        f.write(PING_APP)
    os.system("chmod 755 " + containedapp_file)

    helper.softwarecontainer_iface().Launch(helper.app_id())

    # We need to wait for the ping command to recieve a reply from the remote host
    # But we wait for no more than 10 secs
    timeout = 10
    while helper.poke_softwarecontainer_zombie() == None and timeout > 0:
        time.sleep(1)

    # Return the used config to the test
    return request.param
