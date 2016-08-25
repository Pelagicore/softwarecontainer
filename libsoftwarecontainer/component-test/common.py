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


import commands
import time
import sys
import signal
from subprocess import Popen, call, STDOUT
import os

import dbus
import dbus.service
from dbus.mainloop.glib import DBusGMainLoop

# You must initialize the gobject/dbus support for threading
# before doing anything.
import gobject
gobject.threads_init()

from dbus import glib
glib.init_threads()

IFACE = "com.pelagicore.SoftwareContainer"
OPATH = "/com/pelagicore/SoftwareContainer"
APP_ID = "com.pelagicore.comptest"


class ComponentTestHelper:
    def __init__(self, app_id=APP_ID):
        self.__softwarecontainer_process = None
        self.__softwarecontainer_pid = None
        self.__pc_iface = None
        self.__cookie = self.__generate_cookie()
        self.__app_id = app_id

        print "Generated Cookie = %s, appId = %s" % (self.__cookie, self.__app_id)

        self.__bus = dbus.SessionBus()

        pam_remote_object = self.__bus.get_object("com.pelagicore.PAM", "/com/pelagicore/PAM")
        self.__pam_iface = dbus.Interface(pam_remote_object, "com.pelagicore.PAM")

    def pam_iface(self):
        return self.__pam_iface

    def __generate_cookie(self):
        # Only use the last part, hyphens are not allowed in D-Bus object paths
        return commands.getoutput("uuidgen").strip().split("-").pop()

    def start_softwarecontainer(self, softwarecontainer_bin, container_root,
                            cmd="/controller/controller", suppress_stdout=False):
        """ param  softwarecontainer_bin path to softwarecontainer binary
            param  container_root    path to container root
            param  cmd               command to execute in container

            return true if softwarecontainer started successfully
                   false otherwise
        """
        out = sys.stdout
        if (suppress_stdout):
            out = open(os.devnull, 'wb')
        try:
            self.__softwarecontainer_process = Popen(
                [softwarecontainer_bin, container_root, self.__cookie],
                stdout=out)
        except OSError as e:
            print "Launch error: %s" % e
            return False
        self.__softwarecontainer_pid = self.__softwarecontainer_process.pid
        if self.__find_softwarecontainer_on_dbus() is False:
            print "Could not find SoftwareContainer service on D-Bus"
            return False

        return True

    def is_service_available(self):
        if self.__pc_iface is None:
            return False
        else:
            return True

    def __find_softwarecontainer_on_dbus(self):
        tries = 0
        found = False

        service_name = IFACE + self.__cookie
        while not found and tries < 20:
            try:
                pc_object = self.__bus.get_object(service_name, OPATH)
                self.__pc_iface = dbus.Interface(pc_object, IFACE)
                found = True
            except:
                pass
            time.sleep(1)
            tries = tries + 1
        if found:
            return True
        else:
            return False

    def teardown(self):
        if not self.shutdown_softwarecontainer():
            if not self.__softwarecontainer_pid == 0:
                call(["kill", "-9", str(self.__softwarecontainer_pid)])

    def poke_softwarecontainer_zombie(self):
        """ When using Popen to start a process the process will be a zombie
            when it has exited until the parent process finishes (i.e. the
            test that has an instance of this helper class). This method
            interacts with the process in a way that if it has exited already,
            it will be removed and thus not a zombie anymore. This is useful in
            tests that wants to check if SoftwareContainer is running still.
        """
        returncode = self.__softwarecontainer_process.poll()
        if returncode is not None:
            self.__softwarecontainer_process.communicate()
        return returncode

    def shutdown_softwarecontainer(self):
        if not self.__pc_iface:
            self.__find_softwarecontainer_on_dbus()
            if not self.__pc_iface:
                print "Failed to find softwarecontainer on D-Bus.."
                return False

        print "Shutting down SoftwareContainer"
        try:
            self.__pc_iface.Shutdown()
        except dbus.DBusException as e:
            print "SoftwareContainer already shutdown"
            print "D-Bus says: " + str(e)

        self.__pc_iface = None
        return True

    def softwarecontainer_iface(self):
        return self.__pc_iface

    def app_id(self):
        return self.__app_id

    def cookie(self):
        return self.__cookie

    def softwarecontainer_pid(self):
        return self.__softwarecontainer_pid
