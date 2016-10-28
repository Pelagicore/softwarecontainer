
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


""" This test helper is currently just used by the environment test suite(s).

    The design of it takes height for it being extended and reused by more
    suites. The rationale is that future use-case centric tests will do
    many things basically the same way, and that using specific helpers
    for each suite will create a lot of almost-the-same helpers.

    The intention of this helper going forwards (possibly) is to add command
    line args to be able to configure it runtime (when run in a container)
    to do what is needed. Let's see if future tests can use it like this,
    otherwise it can just be as it is and tied to only the environment test
    suite(s).
"""


# Accessed by users (i.e. tests and test suites)
ENV_VARS_FILE_NAME = "env_vars_dump.json"

# Set to True to have prints from the code
VERBOSE = False


def LOG(message):
    """ To avoid spammy output when needed we wrap the printouts
    """
    if VERBOSE is True:
        print message


class Helper(object):
    """ The helper is run on the inside of containers to investigate
        and interact with from within.

        This means that paths etc. are relative to how the container is
        configured, e.g. when creating it and what gateway configs are used.
    """

    def __init__(self, file_base_path):
        """ file_base_path needs to make sense inside the container
        """
        self.__base_path = file_base_path

    def get_env_vars(self):
        LOG("Getting all env vars")
        all_vars = os.environ
        return all_vars

    def write_result(self, the_type, data):
        LOG("Will write result")
        if the_type == "env_vars":
            LOG("Will dump env vars to disk")
            with open(self.__base_path + "/" + ENV_VARS_FILE_NAME, "w") as fh:
                fh.write(json.dumps(data.data))


def all_env_vars(base_path):
    all_vars = dict()
    with open(base_path + "/" + ENV_VARS_FILE_NAME, "r") as fh:
        all_vars = json.loads(fh.read())
    return all_vars


def env_var(name, base_path):
    all_vars = all_env_vars(base_path)
    return all_vars[name]


if __name__ == "__main__":
    file_base_path = sys.argv[2]

    h = Helper(file_base_path)
    if sys.argv[1] == "--get-env-vars":
        env_vars = h.get_env_vars()
        h.write_result("env_vars", env_vars)
