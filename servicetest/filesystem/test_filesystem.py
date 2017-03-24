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
    return CURRENT_DIR + "/test.log"

DATA = {
    Container.CONFIG: '[{"enableWriteBuffer": false}]',
    Container.BIND_MOUNT_DIR: "/gateways/app",
    Container.HOST_PATH: CURRENT_DIR,
    Container.READONLY: False
}


@pytest.mark.usefixtures("create_testoutput_dir", "dbus_launch", "agent", "assert_no_proxy")
class TestFileSystem(object):
    """ This suite should do some basic testing of the Filesystem within the
        containers and that the whole chain is working properly.
    """

    @pytest.mark.xfail("platform.release() <= \"3.18.0\"")
    @pytest.mark.parametrize("flag", [True, False])
    def test_write_buffer_flag(self, flag):
        """
        Test if the write buffer flag works as expected, if the flag is enabled
        files should not be written directly to the underlying file system. If
        it is disabled, files should be written directly to the file system

        :TODO: If the system doesn't have overlayfs and enableWriteBuffer is
        enabled, this test will work anyways since we fail to mount the fs
        containing fileapp.py, hence failing to create lala.txt, and hence the
        file isn't available after running the create process.

        A way of getting process exit value from the container would be
        very nice.
        """
        absoluteTestFile = os.path.join(CURRENT_DIR, TESTFILE)

        if os.path.exists(absoluteTestFile):
            os.remove(absoluteTestFile)
        ca = Container()
        if flag is True:
            DATA[Container.CONFIG] = '[{"enableWriteBuffer": true}]'
        else:
            DATA[Container.CONFIG] = '[{"enableWriteBuffer": false}]'

        try:
            ca.start(DATA)
            ca.launch_command('{}/fileapp.py create {}'
                              .format(ca.get_bind_dir(), TESTFILE))

            ca.launch_command('{}/fileapp.py check {}'
                              .format(ca.get_bind_dir(), TESTFILE))
            # Give the command time to run inside the container
            time.sleep(0.5)
            # lala.txt should be available in the upper dir, not the lower.
            if flag is True:
                assert os.path.exists(absoluteTestFile) is False
            else:
                assert os.path.exists(absoluteTestFile) is True
            ca.launch_command('{}/fileapp.py delete {}'
                              .format(ca.get_bind_dir(), TESTFILE))
            # lala.txt should be deleted from both the upper and lower dir
            time.sleep(0.5)
            assert os.path.exists(absoluteTestFile) is False
        finally:
            ca.terminate()

    @pytest.mark.xfail("platform.release() <= \"3.18.0\"")
    @pytest.mark.parametrize("flag", [True, False])
    def test_bindmount_files(self, flag):
        """ Test that files are bindmountable as expected.
        """
        absoluteTestFile = os.path.join(CURRENT_DIR, TESTFILE)

        if not os.path.exists(absoluteTestFile):
            f = open(absoluteTestFile, "w")
            f.write("gobbles")
            f.close()

        ca = Container()
        if flag is True:
            DATA[Container.CONFIG] = '[{"enableWriteBuffer": true}]'
        else:
            DATA[Container.CONFIG] = '[{"enableWriteBuffer": false}]'

        try:
            ca.start(DATA)
            ca.bindmount(absoluteTestFile, "/gateways/testfile", True)
            # Give the command time to run inside the container
            time.sleep(0.5)
            assert os.path.exists(absoluteTestFile) is True
            ca.launch_command('{}/fileapp.py check {}'
                              .format(ca.get_bind_dir(), "/gateways/testfile"))
            time.sleep(0.5)
            assert os.path.exists(absoluteTestFile) is True
        finally:
            ca.terminate()

        if os.path.exists(absoluteTestFile):
            os.remove(absoluteTestFile)

    @pytest.mark.xfail("platform.release() <= \"3.18.0\"")
    @pytest.mark.parametrize("flag", [True, False])
    def test_bindmount_fifos(self, flag):
        """ Test that fifos are bindmountable as expected.
        """
        absoluteTestFile = os.path.join("/tmp/", TESTFILE)

        if not os.path.exists(absoluteTestFile):
            os.mkfifo(absoluteTestFile)

        ca = Container()
        if flag is True:
            DATA[Container.CONFIG] = '[{"enableWriteBuffer": true}]'
        else:
            DATA[Container.CONFIG] = '[{"enableWriteBuffer": false}]'

        try:
            ca.start(DATA)
            ca.bindmount(absoluteTestFile, "/gateways/testfile", True)
            # Give the command time to run inside the container
            time.sleep(0.5)
            assert os.path.exists(absoluteTestFile) is True
            ca.launch_command('{}/fileapp.py check {}'
                              .format(ca.get_bind_dir(), "/gateways/testfile"))
            time.sleep(0.5)
            assert os.path.exists(absoluteTestFile) is True
        finally:
            ca.terminate()

        if os.path.exists(absoluteTestFile):
            os.remove(absoluteTestFile)

    @pytest.mark.xfail("platform.release() <= \"3.18.0\"")
    @pytest.mark.parametrize("flag", [True, False])
    def test_bindmount_sockets(self, flag):
        """ Test that sockets are bindmountable as expected.
        """
        absoluteTestFile = os.path.join("/tmp/", TESTFILE)

        if not os.path.exists(absoluteTestFile):
            server = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
            server.bind(absoluteTestFile)

        ca = Container()
        if flag is True:
            DATA[Container.CONFIG] = '[{"enableWriteBuffer": true}]'
        else:
            DATA[Container.CONFIG] = '[{"enableWriteBuffer": false}]'

        try:
            ca.start(DATA)
            ca.bindmount(absoluteTestFile, "/gateways/testfile", True)
            # Give the command time to run inside the container
            time.sleep(0.5)
            assert os.path.exists(absoluteTestFile) is True
            ca.launch_command('{}/fileapp.py check {}'
                              .format(ca.get_bind_dir(), "/gateways/testfile"))
            time.sleep(0.5)
            assert os.path.exists(absoluteTestFile) is True
        finally:
            ca.terminate()

        server.close()

        if os.path.exists(absoluteTestFile):
            os.remove(absoluteTestFile)
