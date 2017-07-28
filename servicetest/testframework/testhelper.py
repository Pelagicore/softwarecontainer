
# Copyright (C) 2016-2017 Pelagicore AB
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

import sys
import os
import json
import argparse
import abc
import resource
import time

""" This test helper is currently just used by the environment test suite(s).

    The design of it takes height for it being extended and reused by more
    suites. The rationale is that future use-case centric tests will do
    many things basically the same way, and that using specific helpers
    for each suite will create a lot of almost-the-same helpers.

    The helper is intended to be run as a program inside a container to drive
    the tests, but also interfaced with directly from the tests when asserting
    values. The code should therefore be considered with these two aspects as
    the same code will have different purpose depending on where it's used.

    The intention of this helper going forwards (possibly) is to add command
    line args to be able to configure it runtime (when run in a container)
    to do what is needed. Let's see if future tests can use it like this,
    otherwise it can just be as it is and tied to only the environment test
    suite(s).
"""


# Set to True to have prints from the code
VERBOSE = False


def LOG(message):
    """ To avoid spammy output when needed we wrap the printouts
    """
    if VERBOSE is True:
        print(message)


class Helper(object):
    """ The helper is run on the inside of containers to investigate the inside
        of the container, and interact with the outside from inside the container.

        This means that paths etc. are relative to how the container is
        configured, e.g. when creating it and what gateway configs are used.

        The current design is assuming that most tests will want the helper to
        write information to a file and later provide that info by reading it
        back.
    """
    __metaclass__ = abc.ABCMeta

    def __init__(self, file_base_path):
        """ file_base_path needs to make sense inside the container.

            This path is either set when the helper is called as a program
            to execute inside the container, or it's set by a test instantiating
            the classes directly when the intention is to read back results from
            the previous run.
        """
        self._base_path = file_base_path


class NetworkHelper(Helper):
    """ Helper for network related things.

        Currently supports running 'ping' and 'ip' from inside the container.
    """

    def __init__(self, file_base_path):
        Helper.__init__(self, file_base_path)

    # Below methods are executed inside container
    def write_result(self, data):
        with open(self._base_path + "/ping_result", "w") as fh:
            fh.write(str(data))
        LOG(data)

    def ping(self, host):
        LOG("pings" + host)
        is_pingable = os.system("ping -c1 " + host)
        return is_pingable

    def ifconfig(self, file_name):
        os.system("ip -4 addr show eth0 | grep inet | awk '{print $2}' >> " + file_name)

    # Below methods are executed on the host (in the tests)
    def compare_ips(self, file1, file2):
        ip1 = None
        ip2 = None
        with open(self._base_path + "/" + file1, "r") as fh:
            ip1 = fh.readline()
        with open(self._base_path + "/" + file2, "r") as fh:
            ip2 = fh.readline()
        return (ip1 == ip2)

    def ping_result(self):
        is_pingable = False
        file_content = None
        with open(self._base_path + "/ping_result", "r") as fh:
            file_content = fh.readline()
        is_pingable = (file_content == "0")
        return is_pingable

    def remove_file(self):
        try:
            os.remove(self._base_path + "/ping_result")
        except:
            LOG("There is no file to remove")

    def remove_files(self, file1, file2):
        os.remove(self._base_path + "/" + file1)
        os.remove(self._base_path + "/" + file2)


class EnvironmentHelper(Helper):
    """ Helper for examining the environment inside the container.
    """

    # Tests access this statically from e.g. fixtures
    ENV_VARS_FILE_NAME = "env_vars_dump.json"

    def __init__(self, file_base_path):
        Helper.__init__(self, file_base_path)

    # Below methods are executed inside container
    def write_result(self, data):
        LOG("Will dump env vars to disk")
        with open(self._base_path + "/" + self.__file_name(), "w") as fh:
            fh.write(json.dumps(data.data))

    def get_env_vars(self):
        LOG("Getting all env vars")
        all_vars = os.environ
        return all_vars

    # Below methods are executed on the host (in the tests)
    def env_var(self, name):
        all_vars = self.__all_env_vars(self._base_path)
        if name in all_vars:
            return all_vars[name]
        else:
            return None

    # Private methods - helpers etc.
    def __all_env_vars(self, base_path):
        all_vars = dict()
        with open(base_path + "/" + self.__file_name(), "r") as fh:
            all_vars = json.loads(fh.read())
        return all_vars

    def __file_name(self):
        """ Provide some de-coupling from the static-ness of the
            file name.
            TODO: This might be unnecessary...
        """
        return EnvironmentHelper.ENV_VARS_FILE_NAME


class CGroupHelper(Helper):
    """ Helper for examining Cgroups Gateway inside the container
    """
    def __init__(self, file_base_path):
        Helper.__init__(self, file_base_path)
        self.__result_file = os.path.join(file_base_path, "cgroup_result")

    # Below methods are executed inside container
    def allocate(self, size):
        """ The main purpose of this function is to allocate memory till it reaches
            maximum allowed threshold and eventully end up OOM Killer to kill process.
        """
        buffer = ""
        step = size / 10
        while (size > len(buffer)):
            try:
                buffer = buffer + ('a' * step)
            except:
                return False

            with open(self.__result_file, "w") as f:
                f.write(str(len(buffer)))

        return True

    # Below methods are executed on the host (in the tests)
    def remove_file(self):
        try:
            os.remove(self.__result_file)
        except:
            LOG("There is no file to remove")

    def result(self):
        file_content = None
        with open(self.__result_file, "r") as fh:
            file_content = fh.read()
        return int(file_content)

class CoreDumpHelper(Helper):
    """ Helper for checking for core dumps
    """

    def __init__(self, file_base_path):
        Helper.__init__(self, file_base_path)

    def checkPattern(self, expected_pattern):
        with open("/proc/sys/kernel/core_pattern", "r") as core_pattern:
            with open(self._base_path + "/core_pattern", "w") as fh:
                pattern = core_pattern.read()
                fh.write(pattern)
                LOG(pattern)

    def patternResult(self):
        with open(self._base_path + "/core_pattern", "r") as fh:
            return fh.read()

    def assertLimit(self):
        (soft, hard) = resource.getrlimit(resource.RLIMIT_CORE)
        with open(self._base_path + "/core_limits", "w") as fh:
            fh.write(str(soft))

    def limitResult(self):
        with open(self._base_path + "/core_limits", "r") as fh:
            return fh.read()

    def dumpCore(self):
        os.abort()

GET_ENV_VARS_OPTION = "get_env_vars"
TEST_DIR_OPTION = "test_dir"
DO_PING_OPTION = "do_ping"
DO_IFCONFIG_OPTION = "do_ifconfig"
DO_ALLOCATE_OPTION = "do_allocate"
DO_CORE_DUMP_OPTION = "core_dump"

if __name__ == "__main__":
    """ When the program is called from command line it is running inside
        a container.
    """

    parser = argparse.ArgumentParser()

    get_env_vars_help_message = """ A path to a file in which to store environment information.
        This file is intended to be used by the helper when reading
        data for asserting values.
    """
    parser.add_argument("--do-get-env-vars",
                        action="store_true",
                        dest=GET_ENV_VARS_OPTION,
                        default=None,
                        help=get_env_vars_help_message)

    test_dir_help_message = """ A path to a file in which to store the result of action
        from inside the container. Either "--do-ping" or "--do_ifconfig"
        action is required to be used in combination to this.
    """
    parser.add_argument("--test-dir",
                        nargs=1,
                        action="store",
                        dest=TEST_DIR_OPTION,
                        default=None,
                        metavar="path",
                        help=test_dir_help_message)

    do_ping_help_message = """ A host name to ping
    """
    parser.add_argument("--do-ping",
                        nargs=1,
                        action="store",
                        dest=DO_PING_OPTION,
                        default=None,
                        metavar="hostname",
                        help=do_ping_help_message)

    do_ifconfig_help_message = """ A path to a file in which to store ip address of the container
    """
    parser.add_argument("--do-ifconfig",
                        nargs=1,
                        action="store",
                        dest=DO_IFCONFIG_OPTION,
                        default=None,
                        metavar="path",
                        help=do_ifconfig_help_message)

    do_allocate_help_message = """ desired size to allocated in container
    """
    parser.add_argument("--do-allocate",
                        nargs=1,
                        action="store",
                        dest=DO_ALLOCATE_OPTION,
                        default=None,
                        metavar="size",
                        help=do_allocate_help_message)

    core_dump_help_message = """ Used to perform core dump related tasks
    """
    parser.add_argument("--core-dump",
                        nargs=1,
                        action="store",
                        dest=DO_CORE_DUMP_OPTION,
                        default=None,
                        metavar="pattern",
                        help=core_dump_help_message)

    args = parser.parse_args()

    parsed_value = getattr(args, TEST_DIR_OPTION,)
    if parsed_value is not None:
        # Extract the actual path string
        test_file_base_path = parsed_value.pop()
        """ This test work with at least two options, first one is base path
            and the second one should be one of below options
            * do_ping
            * do_ifconfig
            * get_env_vars
            * do_allocate
            * wait_for_core_dump
        """
        parsed_value = getattr(args, DO_PING_OPTION)
        if parsed_value is not None:
            # Extract the actual host string
            ping_host = parsed_value.pop()
            h = NetworkHelper(test_file_base_path)
            # Do a ping from inside the container
            is_pingable = h.ping(ping_host)
            # Dump the information for the helper to read back later in the tests
            h.write_result(is_pingable)

        parsed_value = getattr(args, DO_IFCONFIG_OPTION)
        if parsed_value is not None:
            # Extract the actual path string
            if_fname = parsed_value.pop()
            h = NetworkHelper(test_file_base_path)
            # get ip address of container
            h.ifconfig(if_fname)

        parsed_value = getattr(args, GET_ENV_VARS_OPTION)
        if parsed_value is not None:
            h = EnvironmentHelper(test_file_base_path)
            # Get the environment from inside the container
            env_vars = h.get_env_vars()
            # Dump the information for the helper to read back later in the tests
            h.write_result(env_vars)

        parsed_value = getattr(args, DO_ALLOCATE_OPTION)
        if parsed_value is not None:
            # Extract the requested size to allocate
            allocation_size = parsed_value.pop()
            h = CGroupHelper(test_file_base_path)
            # allocate the memory
            success = h.allocate(long(allocation_size))
            LOG("Is allocation successful : " + str(success))

        parsed_value = getattr(args, DO_CORE_DUMP_OPTION)
        if parsed_value is not None:
            expected_pattern = parsed_value.pop()
            h = CoreDumpHelper(test_file_base_path)
            h.checkPattern(expected_pattern)
            h.assertLimit()
            h.dumpCore()

