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


""" The classes in this module can be used directly if needed by the test code,
    but the recommendation is to use the fixtures from 'conftest.py' if there
    are any corresponding to the class needed.
"""

from gi.repository import GObject
import dbus
import json
import time
import threading
import os
import Queue
import subprocess

class Receiver(threading.Thread):
    """ The Receiver class encapsulates and runs a gobject mainloop and dbus implementation in a separate thread

        Purpose of Receiver is to listen to NameOwnerChanged to know about Agent coming and going
    """
    def __init__(self, logFile=None):
        self._logFile = logFile
        threading.Thread.__init__(self)
        GObject.threads_init()
        self.msgQueue = Queue.Queue()

    def log(self, msg):
        if self._logFile is not None:
            self._logFile.write(msg)
        else:
            print(msg)

    def handler(self, gob, gob2, gob3):
        if gob == "com.pelagicore.SoftwareContainerAgent":
            """
            Put softwarecontainerStarted on the message queue, this is picked up by
            other threads which should continue running when softwarecontainer-agent
            is ready to work.
            """
            self.msgQueue.put("softwarecontainerStarted")

    def run(self):
        import dbus.mainloop.glib
        self._gloop = GObject.MainLoop()
        self._loop = dbus.mainloop.glib.DBusGMainLoop()
        self._bus = dbus.SystemBus(mainloop=self._loop)
        self._bus.add_signal_receiver(self.handler, dbus_interface="org.freedesktop.DBus",
                                      signal_name="NameOwnerChanged")
        self._gloop.run()

    def terminate(self):
        if self._loop is not None:
            self._gloop.quit()
            self._gloop = None

    def __del__(self):
        self.terminate()


class ContainerApp():
    """ This represents the container. This can be considered the proxy for the Agent.
    """

    def __init__(self):
        self._path = os.path.dirname(os.path.realpath(__file__))
        self._bus = dbus.SystemBus()
        self._pca_obj = self._bus.get_object("com.pelagicore.SoftwareContainerAgent",
                                             "/com/pelagicore/SoftwareContainerAgent")
        self._pca_iface = dbus.Interface(self._pca_obj, "com.pelagicore.SoftwareContainerAgent")
        self.__bind_dir = None
        self.containerId = None

    def set_host_path(self, path):
        self.__host_path = path

    def __createContainer(self, enableWriteBuffer=False):
        if enableWriteBuffer:
            self.containerId = self._pca_iface.CreateContainer("prefix-dbus-", "{enableWriteBuffer: true}")
        else:
            self.containerId = self._pca_iface.CreateContainer("prefix-dbus-", "")

    def bindMountFolderInContainer(self, dirname):
        return self._pca_iface.BindMountFolderInContainer(self.containerId, self.__host_path, dirname, True)

    def set_gateway_config(self, gateway_id, config):
        self._pca_iface.SetGatewayConfigs(self.containerId, {gateway_id: json.dumps(config)})

    def launchCommand(self, binary):
        response = self._pca_iface.LaunchCommand(self.containerId, 0,
                                                 "{}".format(binary),
                                                 "/gateways/app", "/tmp/stdout", {"": ""})
        if response is -1:
            print "Failed to launch process in container"
            return -1
        return response

    def getBindDir(self):
        return self.__bind_dir

    def start(self, enableWriteBuffer=False):
        self.__createContainer(enableWriteBuffer)
        self.__bind_dir = self.bindMountFolderInContainer("app")

    def terminate(self):
        if self.containerId is not None:
            self._pca_iface.ShutDownContainer(self.containerId)



class SoftwareContainerAgentHandler():
    """ Starts the agent and manages its life cycle, e.g. spawning and killing.

        Used by e.g. ContainerApp over D-Bus.
    """

    def __init__(self, log_file_path=None):
        self.log_file = None
        self.log_file_path = log_file_path
        if self.log_file_path == None:
            self.log_file = subprocess.STDOUT
        else:
            self.log_file = open(log_file_path, "w")
        self.rec = Receiver(logFile=self.log_file)
        self.rec.start()

        # Starting softwarecontainer-agent
        # TODO: This doesn't work if the user pass 'None' as log_file_path
        self.agent = subprocess.Popen("softwarecontainer-agent", stdout=self.log_file, stderr=self.log_file)

        try:
            """
            Wait for the softwarecontainerStarted message to appear on the
            msgQueue, this is evoked when softwarecontainer-agent is ready to
            perform work. If we timeout tear down what we have started so far.
            """
            while self.rec.msgQueue.get(block=True, timeout=5) != "softwarecontainerStarted":
                pass
        except Queue.Empty as e:
            self.agent.terminate()
            self.rec.terminate()
            raise Exception("SoftwareContainer DBus interface not seen", e)

        if self.agent.poll() is not None:
            """
            Make sure we are not trying to perform anything against a dead
            softwarecontainer-agent
            """
            self.rec.terminate()
            raise Exception("SoftwareContainer-agent has died for some reason")

    def terminate(self):
        self.agent.terminate()
        self.rec.terminate()
        self.log_file.close()
