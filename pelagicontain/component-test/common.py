#!/usr/bin/env python

"""
    Copyright (C) 2014 Pelagicore AB
    All rights reserved.
"""

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

IFACE = "com.pelagicore.Pelagicontain"
OPATH = "/com/pelagicore/Pelagicontain"
APP_ID = "com.pelagicore.comptest"


class ComponentTestHelper:
    def __init__(self, app_id=APP_ID):
        self.__pelagicontain_process = None
        self.__pelagicontain_pid = None
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

    def start_pelagicontain(self, pelagicontain_bin, container_root,
                            cmd="/controller/controller", suppress_stdout=False):
        """ param  pelagicontain_bin path to pelagicontain binary
            param  container_root    path to container root
            param  cmd               command to execute in container

            return true if pelagicontain started successfully
                   false otherwise
        """
        out = sys.stdout
        if (suppress_stdout):
            out = open(os.devnull, 'wb')
        try:
            self.__pelagicontain_process = Popen(
                [pelagicontain_bin, container_root, cmd, self.__cookie],
                stdout=out)
        except OSError as e:
            print "Launch error: %s" % e
            return False
        self.__pelagicontain_pid = self.__pelagicontain_process.pid
        if self.__find_pelagicontain_on_dbus() is False:
            print "Could not find Pelagicontain service on D-Bus"
            return False

        return True

    def is_service_available(self):
        if self.__pc_iface is None:
            return False
        else:
            return True

    def __find_pelagicontain_on_dbus(self):
        tries = 0
        found = False

        service_name = IFACE + self.__cookie
        while not found and tries < 2:
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
        if not self.shutdown_pelagicontain():
            if not self.__pelagicontain_pid == 0:
                call(["kill", "-9", str(self.__pelagicontain_pid)])

    def poke_pelagicontain_zombie(self):
        """ When using Popen to start a process the process will be a zombie
            when it has exited until the parent process finishes (i.e. the
            test that has an instance of this helper class). This method
            interacts with the process in a way that if it has exited already,
            it will be removed and thus not a zombie anymore. This is useful in
            tests that wants to check if Pelagicontain is running still.
        """
        returncode = self.__pelagicontain_process.poll()
        if returncode is not None:
            self.__pelagicontain_process.communicate()
        return returncode

    def shutdown_pelagicontain(self):
        if not self.__pc_iface:
            self.__find_pelagicontain_on_dbus()
            if not self.__pc_iface:
                print "Failed to find pelagicontain on D-Bus.."
                return False

        print "Shutting down Pelagicontain"
        try:
            self.__pc_iface.Shutdown()
        except dbus.DBusException as e:
            print "Pelagicontain already shutdown"
            print "D-Bus says: " + str(e)

        self.__pc_iface = None
        return True

    def pelagicontain_iface(self):
        return self.__pc_iface

    def app_id(self):
        return self.__app_id

    def cookie(self):
        return self.__cookie

    def pelagicontain_pid(self):
        return self.__pelagicontain_pid
