#!/usr/bin/env python

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

#
# Don't run this from the scripts/ directory. It is copied into the build dir
# by CMake, and it is there it should be run.
#

import os
import sys

CURRENT_DIR = os.path.dirname(os.path.realpath(__file__))
os.chdir(CURRENT_DIR)

print "### Running tests ###"
gtestFilter = ""
if len(sys.argv) > 1:
    gtestFilter = "--gtest_filter={}".format(sys.argv[1])

testSuites = { }
def runTestSuite(binary, name):
    outputFile = "{}_unittest_result.xml".format(name)
    if os.path.exists(outputFile):
        os.remove(outputFile)

    command = "{} {} --gtest_output=xml:{}".format(binary, gtestFilter, outputFile)
    exitCode = os.system(command)
    testSuites[name] = exitCode

#
# NOTE: Don't add spaces in the names, the names are used as parts of paths to save
#       test results.
#

# These are proper unit tests
runTestSuite("./agent/unit-test/softwarecontaineragent-unit-test", "ConfigStoreTest")
runTestSuite("./common/unit-test/softwarecontainercommon-unit-test", "CommonTest")
runTestSuite("./libsoftwarecontainer/unit-test/softwarecontainer-unit-test", "LibUnitTests")

retval = 0
for name, exitCode in testSuites.items():
    if exitCode is not 0:
        retval = -1
        red = "\033[1;31m"
        reset = "\033[0;0m"
        errorString = "*** The {} failed with exit code {} ***".format(name, exitCode)
        print "{}{}".format(red, "*" * len(errorString))
        print errorString
        print "{}{}".format("*" * len(errorString), reset)

sys.exit(retval)
