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


""" Assert that an environment variable can be set within the container
    by calling SoftwareContainer::SetContainerEnvironmentVariable.
"""

import pytest

import os
import time

# conftest contains some fixtures we use in the tests
import conftest

from common import ComponentTestHelper


# The app writes the environment variables to file so we can check what is set
# inside the container
ENV_APP = """
#!/bin/sh
env > /appshared/env_log
"""

# We keep the helper object module global so external fixtures can access it
helper = ComponentTestHelper()


class TestSetEnvVariables(object):

    def test_pc_should_set_env_variable(self, softwarecontainer_binary,
                                        container_path, teardown_fixture):
        """ Assert SoftwareContainer can set an environment variable inside the
            container.

            * Start PC
            * Set an environment variable
            * Launch an app that writes the current environment variables to file
            * Assert the set variable appears in the file
        """
        variable = "test-var"
        value = "test-val"

        helper.pam_iface().test_reset_values()

        assert helper.start_softwarecontainer(softwarecontainer_binary, container_path)

        pc_iface = helper.softwarecontainer_iface()
        pc_iface.SetContainerEnvironmentVariable(variable, value)
        time.sleep(0.5)
        self.__create_env_app(container_path)
        pc_iface.Launch(helper.app_id())
        time.sleep(0.5)
        assert self.is_env_set(container_path, variable, value)

    def __create_env_app(self, container_path):
        containedapp_file = container_path + "/com.pelagicore.comptest/bin/containedapp"
        with open(containedapp_file, "w") as f:
            print "Overwriting containedapp..."
            f.write(ENV_APP)
        os.system("chmod 755 " + containedapp_file)

    def is_env_set(self, container_path, var, val):
        success = False
        expected = var + "=" + val

        try:
            with open("%s/com.pelagicore.comptest/shared/env_log" % container_path) as f:
                lines = f.readlines()
                for line in lines:
                    if expected in line:
                        success = True
                        break
        except Exception as e:
            pytest.fail("Unable to read command output, output file couldn't be opened! "
                        "Exception: %s" % e)
        return success
