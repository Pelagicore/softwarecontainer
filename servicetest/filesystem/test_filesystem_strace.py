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

import pytest

import os
import time
import platform
import socket
import re
import StringIO


from testframework import Container

CURRENT_DIR = os.path.dirname(os.path.realpath(__file__))
TESTOUTPUT_DIR = CURRENT_DIR + "/testoutput"
TESTFILE = 'testfile.txt'


# This function is used by the test framework to know where test specific
# files should be stored
def output_dir():
    return TESTOUTPUT_DIR


# This function is used by the 'agent' fixture to know where the log should
# be stored
def logfile_path():
    return CURRENT_DIR + "/test-strace.log"

def agent_exec_prefix():
    return "strace"

DATA = {
    Container.CONFIG: '[{"writeBufferEnabled": false}]',
    Container.BIND_MOUNT_DIR: "/gateways/app",
    Container.HOST_PATH: CURRENT_DIR,
    Container.READONLY: False
}

def get_logblock(file, startline, stopline):
    """ This gets a block of text/logs between the startline and stopline
        from a file and returns the text as a string to the caller. Does not
        include startline or stopline in the actual textblock passed to the caller.
    """
    with open(file, "r") as input:
        buf = ""
        for line in input:
            if line.strip() == startline:
                break

        for line in input:
            if line.strip() == stopline:
                break
            buf += line

        return buf


def get_strace_created_files(buf):
    """ The function receives a logblock containing strace log lines and finds
        the possibly created files as well as the created directories. Note that
        open("...", O_CREAT...) also finds already existing files that are just
        opened for write.
    """
    files = list()

    for line in StringIO.StringIO(buf):
        print line
        # Find all open("...", O_CREAT...) instances.
        if re.search('^open(.*)O_CREAT', line):
            p = re.compile('^open\(\"(.*)\",')
            files.append(p.match(line).group(1))
        # Find all mkdir instances.
        if re.search('^mkdir', line):
            p = re.compile('^mkdir\(\"(.*)\",')
            files.append(p.match(line).group(1))

    return files

@pytest.mark.usefixtures("create_testoutput_dir", "dbus_launch", "agent", "assert_no_proxy")
class TestFileSystemStrace(object):
    """ This suite does some strace testing of the creation and deletion of
        containers and make sure we don't create or modify files outside the areas
        we should be working within.

        The tests are performed by prefixing the softwarecontainer-agent binary
        with strace and logging the output into a logfile and then parsing the
        logfile for the files and directories being created. We can then check the
        paths and make sure they are valid.
    """

    @pytest.mark.xfail("platform.release() <= \"3.18.0\"")
    def test_bindmount_sockets_strace(self):
        """ Test that sockets are bindmountable as expected.

            The reason that we chose to use a socket for strace testing is
            completely random and should have no effect on the test as such.
        """
        startword = "======STARTSTARTSTART====="
        endword = "======ENDENDEND====="

        with open(logfile_path(), mode="a") as f:
            f.write("\n" + startword + "\n")

        absolute_test_file = os.path.join("/tmp/", TESTFILE)

        if not os.path.exists(absolute_test_file):
            server = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
            server.bind(absolute_test_file)

        ca = Container()
        DATA[Container.CONFIG] = '[{"writeBufferEnabled": true}]'

        try:
            ca.start(DATA)
            ca.bindmount(absolute_test_file, "/gateways/testfile", True)
            # Give the command time to run inside the container
            time.sleep(0.5)
            assert os.path.exists(absolute_test_file) is True
            ca.launch_command('{}/fileapp.py check {}'
                              .format(ca.get_bind_dir(), "/gateways/testfile"))
            time.sleep(0.5)
            assert os.path.exists(absolute_test_file) is True
        finally:
            ca.terminate()

        server.close()

        if os.path.exists(absolute_test_file):
            os.remove(absolute_test_file)

        time.sleep(0.5)
        with open(logfile_path(), mode="a") as f:
            f.write("\n" + endword + "\n")

        # Get the logblock from the above run containing strace logs from
        # SoftwareContainerAgent
        buf = get_logblock(logfile_path(), startword, endword)

        # Read out the created files and directories from the caught buffer
        files = get_strace_created_files(buf)

        # Directory patterns that are acceptable to create inside.
        patterns = ['/tmp/container/SC-0',
                    '/usr/var/lib/lxc/SC-0',
                    '/sys/fs/cgroup/',
                    '/run/lxc/']

        # Find all instances in files matching any of the patterns and create
        # a fileset containing all "OK" files
        fileset = list()
        for f in set(files):
            for pattern in patterns:
                if pattern in f:
                    fileset.append(f)

        # Append specific hard to match files that will be "created" for
        # various reasons
        fileset.append('/')
        fileset.append('/run/')
        fileset.append('/tmp/stdout')

        # Remove the fileset files from the files list
        for f in fileset:
            files = filter(lambda a: a != f, files)

        # And assert that the list is empty. If anything is left, they are
        # created in a bad location.
        assert files == []
