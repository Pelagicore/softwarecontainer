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


from gi.repository import GObject
import dbus
import json
import time
import threading
import os
import Queue
import subprocess

class Receiver(threading.Thread):
    """
    The Receiver class encapsulates and runs a gobject mainloop and dbus implementation in a separate thread
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
        self._gloop =GObject.MainLoop()
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
    def __init__(self):
        self._path = os.path.dirname(os.path.realpath(__file__))
        self._bus = dbus.SystemBus()
        self._pca_obj = self._bus.get_object("com.pelagicore.SoftwareContainerAgent",
                                             "/com/pelagicore/SoftwareContainerAgent")
        self._pca_iface = dbus.Interface(self._pca_obj, "com.pelagicore.SoftwareContainerAgent")
        self.__bind_dir = None
        self.containerId = None

    def __createContainer(self, enableWriteBuffer=False):
        if enableWriteBuffer:
            self.containerId = self._pca_iface.CreateContainer("prefix-dbus-", "{enableWriteBuffer: true}")
        else:
            self.containerId = self._pca_iface.CreateContainer("prefix-dbus-", "")

    def bindMountFolderInContainer(self, relpath, dirname):
        return self._pca_iface.BindMountFolderInContainer(self.containerId, self._path + relpath, dirname, True)

    def networkGateway(self):
        configuration = {"network": json.dumps([{"internet-access": True, "gateway": "10.0.3.1"}])}
        self._pca_iface.SetGatewayConfigs(self.containerId, configuration)

    def dbusGateway(self):
        connection = os.environ.get("DBUS_SESSION_BUS_ADDRESS")
        configuration = [{
            "dbus-gateway-config-session": [{
                "direction": "*",
                "interface": "*",
                "object-path": "*",
                "method": "*"
            }],
            "dbus-gateway-config-system": []
        }]
        self._pca_iface.SetGatewayConfigs(self.containerId, {"dbus": json.dumps(configuration)})

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
        self.__bind_dir = self.bindMountFolderInContainer("/..", "app")

    def terminate(self):
        if self.containerId is not None:
            self._pca_iface.ShutDownContainer(self.containerId)



class SoftwareContainerAgentHandler():

    def __init__(self, logFile=subprocess.STDOUT):
        self.rec = Receiver(logFile=logFile)
        self.rec.start()

        """ Starting softwarecontainer-agent """
        self.agent = subprocess.Popen("softwarecontainer-agent", stdout=logFile, stderr=logFile)

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
