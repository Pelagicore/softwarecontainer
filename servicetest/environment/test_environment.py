
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

# External utility imports
import os
import time

# Test framework and local test helper imports
import testhelper
from testframework import Container


##### Useful globals #####

CURRENT_DIR = os.path.dirname(os.path.realpath(__file__))
TESTOUTPUT_DIR = CURRENT_DIR + "/testoutput/"


##### Provide what the testframework requires #####

# This function is used by the 'agent' fixture to know where the log should be stored
def logfile_path():
    return TESTOUTPUT_DIR + "/environment-test.log"


##### Local fixtures #####

@pytest.fixture
def clear_env_files(scope="function"):
    """ Removes the files used by the helper to avoid having false
        positives from previous test runs.

        TODO: Should this be a method on the helper?
    """
    file_path = TESTOUTPUT_DIR + testhelper.ENV_VARS_FILE_NAME
    if os.path.exists(file_path):
        os.remove(file_path)


@pytest.fixture
def create_testoutput_dir(scope="module"):
    """ Create a directory for the generated test files.

        This directory is ignored by git but it's nice to have
        somewhere locally to store test output to support
        troubleshooting etc.
    """
    if not os.path.exists(TESTOUTPUT_DIR):
        os.makedirs(TESTOUTPUT_DIR)


##### Globals for setup and configuration of SC #####

# The path to where the test app is located, will be passed in the 'data' dict when
# starting the container. The test app is assumed to be located in the
# same directory as this test.
HOST_PATH = os.path.dirname(os.path.abspath(__file__))

# These default values are used to pass various test specific values and
# configurations to the Container helper. Tests that need to add, remove or
# update entries can simply base their dict on this one for convenience.
DATA = {
    Container.CONFIG: "[{\"enableWriteBuffer\": false}]",
    Container.BIND_MOUNT_DIR: "app",
    Container.HOST_PATH: HOST_PATH
}


#####  Various gateway configs used by the tests. #####
#
# The configs below might be dependent on each other so beware
# when changing them.

GW_CONFIG_ONE_VAR = [
     {
        "name": "MY_ENV_VAR",
        "value": "1234"
    }
]

GW_CONFIG_SAME_VAR = [
    {
        "name": "MY_ENV_VAR",
        "value": "abcd"
    }
]

GW_CONFIG_TWO_VARS = [
     {
        "name": "MY_ENV_VAR",
        "value": "1234"
    },
    {
        "name": "LD_LIBRARY_PATH",
        "value": "/some/non/relevant/path"
    }
]

# This config appends LD_LIBRARY_PATH and should be used
# after e.g. GW_CONFIG_ONE_VAR has been set.
GW_CONFIG_APPEND = [
    {
        "name": "LD_LIBRARY_PATH",
        "value": ":/another/path",
        "append": True
    }
]


##### Test suites #####

@pytest.mark.usefixtures("create_testoutput_dir", "agent", "clear_env_files")
class TestInteractionGatewayAndAPI(object):
    """ This suite tests the interaction between the environment gateway and
        setting of environment variables through the Agent D-Bus API.
    """

    def test_vars_set_by_api_takes_precedence(self):
        """ Setting a varaible through the D-Bus API should mean that value
            takes precedence over a variable previously set by the environment
            gateway.
        """
        try:
            sc = Container()
            sc.start(DATA)
            # Set variable through gateway
            sc.set_gateway_config("env", GW_CONFIG_ONE_VAR)
            # Set the same variable with LaunchCommand
            env_var = {"MY_ENV_VAR": "new-value"}
            sc.launch_command("python " +
                                  sc.get_bind_dir() +
                                  "/testhelper.py --get-env-vars " +
                                  sc.get_bind_dir() + "/testoutput/",
                              env=env_var)

            # The D-Bus LaunchCommand is asynch so let it take effect before assert
            time.sleep(0.5)
            my_env_var = testhelper.env_var("MY_ENV_VAR", TESTOUTPUT_DIR)
            # Assert value is the one set with LaunchCommand
            assert my_env_var == "new-value"
        finally:
            sc.terminate()


@pytest.mark.usefixtures("create_testoutput_dir", "agent", "clear_env_files")
class TestEnvironment(object):
    """ This suite does basic tests on the environment variables inside the
        container.
    """

    def test_setting_same_var_keeps_initial_value(self):
        """ Setting a variable twice should result first value being set and kept.
        """
        try:
            sc = Container()
            sc.start(DATA)
            # Set once
            sc.set_gateway_config("env", GW_CONFIG_ONE_VAR)
            # Set twice
            sc.set_gateway_config("env", GW_CONFIG_SAME_VAR)
            sc.launch_command("python " +
                              sc.get_bind_dir() +
                              "/testhelper.py --get-env-vars " +
                              sc.get_bind_dir() + "/testoutput/")

            # The D-Bus LaunchCommand is asynch so let it take effect before assert
            time.sleep(0.5)
            my_env_var = testhelper.env_var("MY_ENV_VAR", TESTOUTPUT_DIR)
            # Assert value is the one set first
            assert my_env_var == "1234"
        finally:
            sc.terminate()

    def test_appending_then_adding_keeps_initial_value(self):
        """ Append a non existing variable. Then set it per the normal way.
            Assert the value created when "appending" non existing value
            is kept intact.
        """
        try:
            sc = Container()
            sc.start(DATA)
            # Append a variable not set before
            sc.set_gateway_config("env", GW_CONFIG_APPEND)
            # Set the variable that was "appended" previously
            sc.set_gateway_config("env", GW_CONFIG_TWO_VARS)
            sc.launch_command("python " +
                              sc.get_bind_dir() +
                              "/testhelper.py --get-env-vars " +
                              sc.get_bind_dir() + "/testoutput/")

            # The D-Bus LaunchCommand is asynch so let it take effect before assert
            time.sleep(0.5)
            ld_library_path = testhelper.env_var("LD_LIBRARY_PATH", TESTOUTPUT_DIR)
            # Assert the "appended" value is kept
            assert ld_library_path == ":/another/path"
        finally:
            sc.terminate()

    def test_appending_non_existing_var_creates_it(self):
        """ Append to a non existing env var and assert that the variable is created
            as a result.
        """
        try:
            sc = Container()
            sc.start(DATA)
            # Append a variable not set before
            sc.set_gateway_config("env", GW_CONFIG_APPEND)
            sc.launch_command("python " +
                              sc.get_bind_dir() +
                              "/testhelper.py --get-env-vars " +
                              sc.get_bind_dir() + "/testoutput/")

            # The D-Bus LaunchCommand is asynch so let it take effect before assert
            time.sleep(0.5)
            variable = testhelper.env_var("LD_LIBRARY_PATH", TESTOUTPUT_DIR)
            # Assert the append resulted in the creation of the variable
            assert variable == ":/another/path"
        finally:
            sc.terminate()

    def test_env_var_is_appended(self):
        """ Set a config and then an appending config, assert the appended
            variable looks as expected.
        """
        try:
            sc = Container()
            sc.start(DATA)
            # Set a config to append to
            sc.set_gateway_config("env", GW_CONFIG_TWO_VARS)
            # Append the previously set variable
            sc.set_gateway_config("env", GW_CONFIG_APPEND)
            sc.launch_command("python " +
                              sc.get_bind_dir() +
                              "/testhelper.py --get-env-vars " +
                              sc.get_bind_dir() + "/testoutput/")

            # The D-Bus LaunchCommand is asynch so let it take effect before assert
            time.sleep(0.5)
            ld_library_path = testhelper.env_var("LD_LIBRARY_PATH", TESTOUTPUT_DIR)
            # Assert the append was applied
            assert ld_library_path == "/some/non/relevant/path:/another/path"
            my_env_var = testhelper.env_var("MY_ENV_VAR", TESTOUTPUT_DIR)
            # Assert other variables were not affected
            assert my_env_var == "1234"
        finally:
            sc.terminate()

    def test_env_var_is_set(self):
        """ Set an Environment gateway config that sets a variable inside
            the container. Use helper to get the inside environment and
            assert the expected result.
        """
        try:
            sc = Container()
            sc.start(DATA)
            sc.set_gateway_config("env", GW_CONFIG_ONE_VAR)
            sc.launch_command("python " +
                              sc.get_bind_dir() +
                              "/testhelper.py --get-env-vars " +
                              sc.get_bind_dir() + "/testoutput/")

            # The D-Bus LaunchCommand is asynch so let it take effect before assert
            time.sleep(0.5)
            assert testhelper.env_var("MY_ENV_VAR", TESTOUTPUT_DIR) == "1234"
        finally:
            sc.terminate()

    def test_multiple_env_vars_are_set(self):
        """ Set an Environment gateway config that sets multiple variables inside
            the container. Use helper to get the inside environment and
            assert the expected reisult.
        """
        try:
            sc = Container()
            sc.start(DATA)
            sc.set_gateway_config("env", GW_CONFIG_TWO_VARS)
            sc.launch_command("python " +
                              sc.get_bind_dir() +
                              "/testhelper.py --get-env-vars " +
                              sc.get_bind_dir() + "/testoutput/")

            # The D-Bus LaunchCommand is asynch so let it take effect before assert
            time.sleep(0.5)
            assert testhelper.env_var("MY_ENV_VAR", TESTOUTPUT_DIR) == "1234"
            assert testhelper.env_var("LD_LIBRARY_PATH", TESTOUTPUT_DIR) == "/some/non/relevant/path"
        finally:
            sc.terminate()
