#!/usr/bin/env python

"""
    Copyright (C) 2014 Pelagicore AB
    All rights reserved.
"""


import commands, os, time, sys, signal
from subprocess import Popen, call, check_output
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
        # Some globals used throughout the tests
        self.pelagicontain_pid = None
        self.pelagicontain_iface = None

        # Result flag is set to non zero on failure. Return value is set as exit
        # status and is used by e.g. ctest to determine test results.
        self.result = 0

        self.cookie = self.generate_cookie()
        self.app_uuid = "com.pelagicore.comptest"
        print "Generated Cookie = %s, appId = %s" % (self.cookie, self.app_uuid)

        self.bus = self.create_session_bus()
        
        pam_remote_object = self.bus.get_object("com.pelagicore.PAM", "/com/pelagicore/PAM")
        self.pam_iface = dbus.Interface(pam_remote_object, "com.pelagicore.PAM")

        self.container_root_dir = "/tmp/test/"
        self.pelagicontain_binary = self.pelagicontain_binary_path()

    def create_session_bus(self):
        return dbus.SessionBus()

    def pelagicontain_binary_path(self):
        # Check if user pointed out a path to Pelagicontain
        pelagicontain_binary = ""
        if os.environ.has_key("PC_BINARY"):
            pelagicontain_binary = str(os.environ["PC_BINARY"])
            if not pelagicontain_binary.endswith("pelagicontain"):
                if not pelagicontain_binary.endswith("/"):
                    pelagicontain_binary += "/"
                pelagicontain_binary += "pelagicontain"
        else:
            pelagicontain_binary = "pelagicontain"

        return pelagicontain_binary

    def pam_iface(self):        
        return self.pam_iface

    def generate_cookie(self):
        # Only use the last part, hyphens are not allowed in D-Bus object paths
        return commands.getoutput("uuidgen").strip().split("-").pop()
    
    def generate_app_uuid(self):
        return commands.getoutput("uuidgen").strip()

    def start_pelagicontain(self, command):
        # The intention is to pass a cookie to Pelagicontain which it will use to
        # destinguish itself on D-Bus (as we will potentially have multiple instances
        # running in the system
        return self.start_pelagicontain2(self.pelagicontain_binary,
                                         self.container_root_dir, command)

    def start_pelagicontain2(self, pelagicontain_bin, container_root, cmd):
        # @param  pelagicontain_bin path to pelagicontain binary
        # @param  container_root    path to container root
        # @param  cmd               command to execute in container
        # @return true if pelagicontain started successfully
        #         false otherwise
        try:
            self.pelagicontain_pid = Popen([pelagicontain_bin, container_root,
                                            cmd, self.cookie]).pid
        except OSError as e:
            return False

        return True

    def find_pelagicontain_on_dbus(self):
        tries = 0
        found = False
        while not found and tries < 2:
            try:
                self.pelagicontain_remote_object = \
                    self.bus.get_object("com.pelagicore.Pelagicontain",
                                   "/com/pelagicore/Pelagicontain/" + self.cookie)
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

    def cleanup_and_finish(self):
        if not self.pelagicontain_pid == 0:
            call(["kill", "-9", str(self.pelagicontain_pid)])
        if self.result == 0:
            print "\n-=< All tests passed >=-\n"
        else:
            print "\n-=< Failure >=-\n"
        exit(self.result)

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

    # ----------------------------- Shutdown
    """ Calling Shutdown will raise a D-Bus error since Pelagicontain shuts down
        without sending a reply. D-Bus seems to consider it an error that there is
        no reply even if the method itself is not supposed to return anything. We
        catch the exception and ignore it.
    """
    def shutdown_pelagicontain(self):
        if not self.pelagicontain_iface:
            self.find_pelagicontain_on_dbus()
            if not self.pelagicontain_iface:
                print "Failed to find pelagicontain on D-Bus.."
                return False
        self.pelagicontain_iface.Shutdown()
        print "Shutting down Pelagicontain"

        return True
