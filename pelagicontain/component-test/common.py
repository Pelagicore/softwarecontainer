#!/usr/bin/env python

"""
    Copyright (C) 2014 Pelagicore AB
    All rights reserved.
"""


import commands, os, time, sys, signal, sys
from subprocess import Popen, call, check_output, STDOUT
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


class ComponentTestHelper:
    def __init__(self):
        self.pelagicontain_pid = None
        self.pelagicontain_iface = None

        self.cookie = self.generate_cookie()
        self.app_uuid = "com.pelagicore.comptest"
        print "Generated Cookie = %s, appId = %s" % (self.cookie, self.app_uuid)

        self.bus = self.create_session_bus()

        pam_remote_object = self.bus.get_object("com.pelagicore.PAM", "/com/pelagicore/PAM")
        self.__pam_iface = dbus.Interface(pam_remote_object, "com.pelagicore.PAM")

    def create_session_bus(self):
        return dbus.SessionBus()

    def pam_iface(self):
        return self.__pam_iface

    def generate_cookie(self):
        # Only use the last part, hyphens are not allowed in D-Bus object paths
        return commands.getoutput("uuidgen").strip().split("-").pop()

    def generate_app_uuid(self):
        return commands.getoutput("uuidgen").strip()


    def start_pelagicontain(self, pelagicontain_bin, container_root, cmd,
                             suppress_stdout=False):
        # @param  pelagicontain_bin path to pelagicontain binary
        # @param  container_root    path to container root
        # @param  cmd               command to execute in container
        # @return true if pelagicontain started successfully
        #         false otherwise
        out = sys.stdout
        if (suppress_stdout):
            out = open(os.devnull, 'wb')
        try:
            self.pelagicontain_pid = Popen([pelagicontain_bin, container_root,
                                            cmd, self.cookie], stdout=out).pid
        except OSError as e:
            print "Launch error: %s" % e
            return False

        return True

    def find_pelagicontain_on_dbus(self):
        tries = 0
        found = False

        while not found and tries < 20:
            try:
                self.pelagicontain_remote_object = \
                    self.bus.get_object("com.pelagicore.Pelagicontain" + self.cookie,
                                   "/com/pelagicore/Pelagicontain")
                self.pelagicontain_iface =\
                    dbus.Interface(self.pelagicontain_remote_object,
                                   "com.pelagicore.Pelagicontain")
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
            if not self.pelagicontain_pid == 0:
                call(["kill", "-9", str(self.pelagicontain_pid)])

    def find_and_run_Launch_on_pelagicontain_on_dbus(self):
        self.pelagicontain_iface = dbus.Interface(self.pelagicontain_remote_object, 
                                             "com.pelagicore.Pelagicontain")
        try:
            self.pelagicontain_iface.Launch(self.app_uuid)
            return True
        except Exception as e:
            print e
            return False

    def make_system_call(self, cmds):
        try:
            output = check_output(cmds)
            return (True, output)
        except:
            return (False, "")

    def shutdown_pelagicontain(self):
        if not self.pelagicontain_iface:
            self.find_pelagicontain_on_dbus()
            if not self.pelagicontain_iface:
                print "Failed to find pelagicontain on D-Bus.."
                return False

        print "Shutting down Pelagicontain"
        try:
            self.pelagicontain_iface.Shutdown()
        except dbus.DBusException as e:
            print "Pelagicontain already shutdown"
            print "D-Bus says: " + str(e)

        self.pelagicontain_iface = None
        return True
