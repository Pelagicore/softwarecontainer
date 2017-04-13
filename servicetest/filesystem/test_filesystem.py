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
import psutil
from dbus.exceptions import DBusException

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
    Container.CONFIG: '[{"writeBufferEnabled": false}]',
    Container.BIND_MOUNT_DIR: "/gateways/app",
    Container.HOST_PATH: CURRENT_DIR,
    Container.READONLY: False
}

TenMB = 10485760


@pytest.mark.usefixtures("create_testoutput_dir", "dbus_launch", "agent",
                         "assert_no_proxy")
class TestFileSystem(object):
    """ This suite should do some basic testing of the Filesystem within the
        containers and that the whole chain is working properly.
    """

    @pytest.mark.xfail("platform.release() <= \"3.18.0\"")
    @pytest.mark.parametrize("flag", [True, False])
    def test_write_buffer_flag(self, flag):
        """ Test if the write buffer flag works as expected, if the flag is
            enabled files should not be written directly to the underlying
            file system. If it is disabled, files should be written directly
            to the file system

            :TODO: If the system doesn't have overlayfs and writeBufferEnabled
            is enabled, this test will work anyways since we fail to mount the
            fs containing fileapp.py, hence failing to create lala.txt, and
            hence the file isn't available after running the create process.

            A way of getting process exit value from the container would be
            very nice.
        """
        absoluteTestFile = os.path.join(CURRENT_DIR, TESTFILE)

        if os.path.exists(absoluteTestFile):
            os.remove(absoluteTestFile)
        ca = Container()
        if flag is True:
            DATA[Container.CONFIG] = '[{"writeBufferEnabled": true}]'
        else:
            DATA[Container.CONFIG] = '[{"writeBufferEnabled": false}]'

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
            DATA[Container.CONFIG] = '[{"writeBufferEnabled": true}]'
        else:
            DATA[Container.CONFIG] = '[{"writeBufferEnabled": false}]'

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
            DATA[Container.CONFIG] = '[{"writeBufferEnabled": true}]'
        else:
            DATA[Container.CONFIG] = '[{"writeBufferEnabled": false}]'

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
            DATA[Container.CONFIG] = '[{"writeBufferEnabled": true}]'
        else:
            DATA[Container.CONFIG] = '[{"writeBufferEnabled": false}]'

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

    @pytest.mark.xfail("platform.release() <= \"3.18.0\"")
    def test_tmpfs_writebuffer_size_enabled(self):
        """ The SoftwareContainer will create a tmpfs in the location of the
            containers temporary directory location if the write buffer is
            enabled. Test that the tmpfs mounted in the containers temporary
            directory structure works as expected.

            Note: The filesystem is defaults to 100MB, but is be configurable
            via the DATA field. This tests the default value.
        """

        absoluteTestFile = None

        ca = Container()

        DATA[Container.CONFIG] = '[{"writeBufferEnabled": true}]'

        try:
            id = ca.start(DATA)

            absoluteTestFile = os.path.join("/tmp/container/SC-" + str(id) +
                                            "/rootfs-upper",
                                            TESTFILE)

            partitions = psutil.disk_partitions(all=True)
            interesting_partition = False
            for part in partitions:
                if part.mountpoint == '/tmp/container/SC-' + str(id):
                    interesting_partition = part
                    break
            assert interesting_partition is not False

            # Try to write 95 megabytes, this should be possible since the
            # default size of the tmpfs is 100 megabyte.
            with open(absoluteTestFile, "w") as f:
                f.write("w" * 95 * 1024 * 1024)

            # Make sure the file is between 94 and 96 megabyte (exact size
            # may vary depending on platform, blocksize etc)
            assert os.path.getsize(absoluteTestFile) >= 94 * 1024 * 1024
            assert os.path.getsize(absoluteTestFile) <= 96 * 1024 * 1024

            # Try to write a file that is too large for the filesystem (105
            # megabytes). This should fail with an IOError.
            with open(absoluteTestFile, "w") as f:
                with pytest.raises(IOError):
                    f.write("w" * 105 * 1024 * 1024)
        finally:
            ca.terminate()

        if os.path.exists(absoluteTestFile):
            os.remove(absoluteTestFile)

    @pytest.mark.xfail("platform.release() <= \"3.18.0\"")
    def test_tmpfs_writebuffer_size_enabled_10mb(self):
        """ The SoftwareContainer will create a tmpfs in the location of the
            containers temporary directory location if the write buffer is
            enabled. Test that the tmpfs mounted in the containers temporary
            directory structure works as expected.

            This test will create a 10 MB tmpfs which will be tested that it
            behaves as expected. Files larger than 10MB can not be written and
            files smaller than 9MB can be written.
        """

        absoluteTestFile = None

        ca = Container()

        DATA[Container.CONFIG] = '[{"writeBufferEnabled": true, \
                                "temporaryFileSystemWriteBufferEnabled": true,\
                                "temporaryFileSystemSize": ' + str(TenMB) + '}]'
        try:
            id = ca.start(DATA)
            absoluteTestFile = os.path.join("/tmp/container/SC-" + str(id) +
                                            "/rootfs-upper",
                                            TESTFILE)
            partitions = psutil.disk_partitions(all=True)
            interesting_partition = False
            for part in partitions:
                if part.mountpoint == '/tmp/container/SC-' + str(id):
                    interesting_partition = part
                    break
            assert interesting_partition is not False

            # Write a 9 MB file and make sure it is in the right size area (due
            # to how filesystems works, it may be a bit bigger or smaller so
            # exact comparison is not possible)
            with open(absoluteTestFile, "w") as f:
                f.write("w" * 9 * 1024 * 1024)

            # As mentioned exact comparison is not possible, so approximately
            # right at least....
            assert os.path.getsize(absoluteTestFile) >= 8 * 1024 * 1024
            assert os.path.getsize(absoluteTestFile) <= 10 * 1024 * 1024

            # Try to write a file that is too big and it should raise an
            # IOError exception.
            with open(absoluteTestFile, "w") as f:
                with pytest.raises(IOError):
                    f.write("w" * 105 * 1024 * 1024)
        finally:
            ca.terminate()

        if os.path.exists(absoluteTestFile):
            os.remove(absoluteTestFile)

    @pytest.mark.xfail("platform.release() <= \"3.18.0\"")
    def test_tmpfs_writebuffer_size_missing_size_config(self):
        """ This test will test various bad configuration combinations and
            make sure that SoftwareContainerAgent behaves as expected when
            the bad configuration is sent to it.

            This tests that the container is created even if the size parameter
            is missing from the configuration.
        """
        ca = Container()

        absoluteTestFile = None

        DATA[Container.CONFIG] = '[{"writeBufferEnabled": true, \
                               "temporaryFileSystemWriteBufferEnabled": true}]'

        try:
            id = ca.start(DATA)

            absoluteTestFile = os.path.join("/tmp/container/SC-" + str(id) +
                                            "/",
                                            TESTFILE)

            with open(absoluteTestFile, "w") as f:
                f.write("w" * 95 * 1024 * 1024)

            # Make sure the file is between 94 and 96 megabyte (exact size
            # may vary depending on platform, blocksize etc)
            assert os.path.getsize(absoluteTestFile) >= 94 * 1024 * 1024
            assert os.path.getsize(absoluteTestFile) <= 96 * 1024 * 1024

            # Try to write a file that is too large for the filesystem (105
            # megabytes). This should fail with an IOError.
            with open(absoluteTestFile, "w") as f:
                with pytest.raises(IOError):
                    f.write("w" * 105 * 1024 * 1024)
        finally:
            ca.terminate()

    @pytest.mark.xfail("platform.release() <= \"3.18.0\"")
    def test_tmpfs_writebuffer_size_missing_tmpfs_enable_flag(self):
        """ This test will test various bad configuration combinations and
            make sure that SoftwareContainerAgent behaves as expected when
            the bad configuration is sent to it.

            This run should work, but no temporary filesystem will be setup as
            the temporaryFileSystemWriteBufferEnabled defaults to false and no
            parsing will be performed on the temporaryFileSystemSize parameter.
        """
        ca = Container()

        absoluteTestFile = None

        DATA[Container.CONFIG] = '[{"writeBufferEnabled": true, \
                               "temporaryFileSystemSize": ' + str(TenMB) + '}]'

        try:
            id = ca.start(DATA)

            absoluteTestFile = os.path.join("/tmp/container/SC-" + str(id) +
                                            "/",
                                            TESTFILE)

            with open(absoluteTestFile, "w") as f:
                f.write("w" * 95 * 1024 * 1024)

            # Make sure the file is between 94 and 96 megabyte (exact size
            # may vary depending on platform, blocksize etc)
            assert os.path.getsize(absoluteTestFile) >= 94 * 1024 * 1024
            assert os.path.getsize(absoluteTestFile) <= 96 * 1024 * 1024

            # Try to write a file that is too large for the filesystem (105
            # megabytes). This should fail with an IOError.
            with open(absoluteTestFile, "w") as f:
                with pytest.raises(IOError):
                    f.write("w" * 105 * 1024 * 1024)
        finally:
            ca.terminate()

    @pytest.mark.xfail("platform.release() <= \"3.18.0\"")
    def test_tmpfs_writebuffer_size_disabled(self):
        """ The SoftwareContainer will create a tmpfs in the location of the
            containers temporary directory location if the write buffer is
            enabled. Test that the tmpfs mounted in the containers temporary
            directory structure works as expected when the writebuffer is
            disabled.
        """

        absoluteTestFile = None

        ca = Container()

        DATA[Container.CONFIG] = '[{"writeBufferEnabled": false}]'

        try:
            id = ca.start(DATA)
            absoluteTestFile = os.path.join("/tmp/container/SC-" + str(id) +
                                            "/",
                                            TESTFILE)

            with open(absoluteTestFile, "w") as f:
                f.write("w" * 105 * 1024 * 1024)
        finally:
            ca.terminate()

        if os.path.exists(absoluteTestFile):
            os.remove(absoluteTestFile)
