
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

import sys
import os
import json
import argparse
import abc


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
        print message


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

    @abc.abstractmethod
    def write_result(self, data):
        """ Write data to file.

            What this method actually does is specific for each inheriting class,
            which should all implement this method.
        """
        return


class EnvironmentHelper(Helper):
    """ Helper for examining the environment inside the container.
    """

    # Tests access this statically from e.g. fixtures
    ENV_VARS_FILE_NAME = "env_vars_dump.json"

    def __init__(self, file_base_path):
        Helper.__init__(self, file_base_path)

    #### Below methods are executed inside container
    def write_result(self, data):
        LOG("Will dump env vars to disk")
        with open(self._base_path + "/" + self.__file_name(), "w") as fh:
            fh.write(json.dumps(data.data))

    def get_env_vars(self):
        LOG("Getting all env vars")
        all_vars = os.environ
        return all_vars

    #### Below methods are executed on the host (in the tests)
    def env_var(self, name):
        all_vars = self.__all_env_vars(self._base_path)
        return all_vars[name]

    #### Private methods - helpers etc.
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


GET_ENV_VARS_OPTION = "get_env_vars"

if __name__ == "__main__":
    """ When the program is called from command line it is running inside
        a container.
    """

    parser = argparse.ArgumentParser()

    get_env_vars_help_message = \
    """ A path to a file in which to store environment information.
        This file is intended to be used by the helper when reading
        data for asserting values.
    """
    parser.add_argument("--get-env-vars",
                        nargs=1,
                        action="store",
                        dest=GET_ENV_VARS_OPTION,
                        default=None,
                        metavar="path",
                        help=get_env_vars_help_message)

    args = parser.parse_args()

    file_base_path_parsed_value = getattr(args, GET_ENV_VARS_OPTION)
    if file_base_path_parsed_value is not None:
        # Extract the actual path string
        file_base_path = file_base_path_parsed_value.pop()
        h = EnvironmentHelper(file_base_path)
        # Get the environment from inside the container
        env_vars = h.get_env_vars()
        # Dump the information for the helper to read back later in the tests
        h.write_result(env_vars)
