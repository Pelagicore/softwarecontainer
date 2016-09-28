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

import os
import time
import subprocess
import fileapp
import time

from testframework import Container

CURRENT_DIR = os.path.dirname(os.path.realpath(__file__))


# This function is used by the 'agent' fixture to know where the log should be stored
def logfile_path():
    return CURRENT_DIR + "/test.log"


DATA = {
    Container.PREFIX: "dbus-test-",
    Container.CONFIG: '[{"enableWriteBuffer": false}]',
    Container.BIND_MOUNT_DIR: "app",
    Container.HOST_PATH: CURRENT_DIR
}


@pytest.mark.usefixtures("dbus_launch", "agent", "assert_no_proxy")
class TestFileSystem(object):
    """ This suite should do some basic testing of the Filesystem within the
    containers and that the whole chain is working properly.
    """

    def test_enable_write_buffer_flag(self):
        if os.path.exists(CURRENT_DIR + '/lala.txt'):
            os.remove(CURRENT_DIR + '/lala.txt')
        ca = Container()
        DATA[Container.CONFIG] = '[{"enableWriteBuffer": true}]'
        try:
            ca.start(DATA)
            ca.launch_command('{}/fileapp.py create lala.txt'.format(ca.get_bind_dir()))
            ca.launch_command('{}/fileapp.py check lala.txt'.format(ca.get_bind_dir()))
            # Give the command time to run inside the container
            time.sleep(0.5)
            # lala.txt should be available in the upper dir, not the lower.
            assert os.path.exists(CURRENT_DIR + '/lala.txt') is False
            ca.launch_command('{}/fileapp.py delete lala.txt'.format(ca.get_bind_dir()))
            # lala.txt should be deleted from both the upper and lower dir
            assert os.path.exists(CURRENT_DIR + '/lala.txt') is False
        finally:
            ca.terminate()

    def test_disable_write_buffer_flag(self):
        if os.path.exists(CURRENT_DIR + '/lala.txt'):
            os.remove(CURRENT_DIR + '/lala.txt')
        ca = Container()
        DATA[Container.CONFIG] = '[{"enableWriteBuffer": false}]'
        try:
            ca.start(DATA)
            ca.launch_command('{}/fileapp.py create lala.txt'.format(ca.get_bind_dir()))
            # Give the command time to run inside the container
            time.sleep(0.5)
            # lala.txt should be available in the app dir
            assert os.path.exists(CURRENT_DIR + '/lala.txt') is True
            ca.launch_command('{}/fileapp.py check lala.txt'.format(ca.get_bind_dir()))
            ca.launch_command('{}/fileapp.py delete lala.txt'.format(ca.get_bind_dir()))
            # Give the command time to run inside the container
            time.sleep(0.5)
            # lala.txt should be deleted from the app dir
            assert os.path.exists(CURRENT_DIR + '/lala.txt') is False
        finally:
            ca.terminate()
