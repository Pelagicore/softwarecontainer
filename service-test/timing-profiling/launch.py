#!/usr/bin/env python

"""
Introduction
============
This test suite is used to measure timing values between different parts of
pelagicontain, both inside the core application, and outside it. The suite sets
up all the necessary components to interact with pelagicontain-agent, and issues
commands to it using dbus.


Goals
=====
* Run pelagicontain-agent and get profiling values from it into a log file
* Run several apps inside containers using pelagicontain-agent and get profiling
  values from these runs.
* Terminate everything and get profiling values from this.

Architecture
============
Calling this architecture might be a bit of an overstatement. The test suite spawns
a Receiver thread for dbus which will put "messages of interest" received on dbus on
the msgQueue, which will be picked up by the main test.

Each App is started using the ContainerApp objects which is basically just a
convenience class wrapping up some dbus functionality.

Once all apps have been spun up and are running, they are terminated and
pelagicontain-agent is spun down as well as the Receiver.

All the output plus some extra data from the Receiver is logged to a physical file.
When everything is finished, we go through the file to gather pertinent data, and do
measurements and output it to files as expected by Jenkins.

"""

import gobject
import dbus
import time
import threading
import os
import subprocess
import Queue
import sys
import tempfile
import string
import re


msgQueue = Queue.Queue()


class Receiver(threading.Thread):
    """
    The Receiver class encapsulates and runs a gobject mainloop and dbus implementation in a separate thread
    """
    def __init__(self, logFile=None):
        self._logFile = logFile
        threading.Thread.__init__(self)
        gobject.threads_init()

    def log(self, msg):
        if self._logFile is not None:
            self._logFile.write(msg)
        else:
            print(msg)

    def handler(self, gob, gob2, gob3):
        self.log("pythonProfilingPoint dbusAvailable %.09f %s %s %s" % (time.time(), gob, gob2, gob3))
        if gob == "com.pelagicore.PelagicontainAgent":
            """
            Put pelagicontainStarted on the message queue, this is picked up by
            other threads which should continue running when pelagicontain-agent
            is ready to work.
            """
            msgQueue.put("pelagicontainStarted")

    def run(self):
        import dbus.mainloop.glib
        self._gloop = gobject.MainLoop()
        self._loop = dbus.mainloop.glib.DBusGMainLoop()
        self._bus = dbus.SystemBus(mainloop=self._loop)
        self._bus.add_signal_receiver(self.handler, dbus_interface="org.freedesktop.DBus", signal_name="NameOwnerChanged")
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
        self._pca_obj = self._bus.get_object("com.pelagicore.PelagicontainAgent", "/com/pelagicore/PelagicontainAgent")
        self._pca_iface = dbus.Interface(self._pca_obj, "com.pelagicore.PelagicontainAgent")

    def createContainer(self):
        self.containerId = self._pca_iface.CreateContainer("prefix")

    def bindMountFolderInContainer(self):
        self._pca_iface.BindMountFolderInContainer(self.containerId, self._path, "app", True)

    def launchCommand(self):
        if self._pca_iface.LaunchCommand(self.containerId, 0, "/gateways/app/simple", "/gateways/app", "/tmp/stdout", {"": ""}) is -1:
            print "Failed to launch process in container"
            return -1

    def shutdown(self):
        self._pca_iface.ShutDownContainer(self.containerId)

    def start(self):
        self.createContainer()
        self.bindMountFolderInContainer()
        self.launchCommand()

    def terminate(self):
        self.shutdown()


def runTest(numStarts=3, logFile=None):

        rec = Receiver(logFile=logFile)
        rec.start()

        time.sleep(0.5)

        print "Start pelagicontain-agent"
        agent = subprocess.Popen("pelagicontain-agent", stdout=logFile, stderr=logFile)

        try:
            """
            Wait for hte pelagicontainStarted message to appear on the
            msgQueue, this is evoked when pelagicontain-agent is ready to
            perform work. If we timeout tear down what we have started so far.
            """
            while msgQueue.get(block=True, timeout=5) != "pelagicontainStarted":
                pass
        except Queue.Empty as e:
            print "Pelagicontain DBus interface not seen"
            agent.terminate()
            rec.terminate()
            sys.exit(-1)

        if agent.poll() is not None:
            """
            Make sure we are not trying to perform anything against a dead
            pelagicontain-agent
            """
            print "Pelagicontain-agent has died for some reason"
            rec.terminate()
            sys.exit(-1)

        apps = []
        for app in range(0, numStarts):
            """
            Start numStarts apps in pelagicontain.
            """
            print "Start app " + str(app)
            container = ContainerApp()
            container.start()
            apps.append(container)

        time.sleep(5)

        for app in apps:
            """
            Tear down all started apps
            """
            app.terminate()

        agent.terminate()
        rec.terminate()


def removeAnsi(astring):
    ansi_escape = re.compile(r'\x1b[^m]*m')
    return ansi_escape.sub('', astring)


def getPythonPoint(logFile, pointName, matchNumber=1):
    """
    This function will grab the timestamp from a python log entry with pointName in the
    logFile
    """
    match = 0
    logFile.seek(0)
    for line in logFile:
        if re.search(pointName, line):
            match = match + 1
            if match == matchNumber:
                print removeAnsi(string.split(line)[2]) + "\n" + line
                return removeAnsi(string.split(line)[2])
    print "Nothing found! " + pointName + " " + str(matchNumber)
    return 0


def getLogPoint(logFile, pointName, matchNumber=1):
    """
    This funciton will grab a timestamp from logFile with the entry name poointName, if no
    matchNumber is declared, it will return the first found entry.

    matchNumber can be used to set which entry to pick up, when multiple entries are found.
    This might useful when starting multiple apps for example.
    """
    match = 0
    logFile.seek(0)
    for line in logFile:
        if pointName in line:
            match = match + 1
            print "Got match " + str(match)
            if match == matchNumber:
                print removeAnsi(string.split(line)[4]) + "\n" + line
                return removeAnsi(string.split(line)[4])
    print "Nothing found! " + pointName + " " + str(matchNumber)
    return 0


def writeMeasurement(fileName, value, url=None):
    """
    Write a measurement file to fileName containing the value and url entry.
    This is for the properties files as defined in Jenkins Plot Plugin
    """
    f = open(fileName, 'w')
    f.write("YVALUE=" + str(value) + "\n")
    if url is not None:
        f.write("URL=" + url + "\n")
    f.close()


def measure(logFile):
    if logFile is None:
        return False
    for line in logFile:
        print line

    start = "softwareContainerStart"
    end = "dbusAvailable"
    writeMeasurement("result-" + start + "-" + end + "-1.properties",
                     float(getPythonPoint(logFile, end)) - float(getLogPoint(logFile, start)))

    for i in range(1, 4):
        start = "createContainerStart"
        end = "launchCommandEnd"
        writeMeasurement("result-" + start + "-" + end + "-" + str(i) + ".properties",
                         float(getLogPoint(logFile, end, i)) - float(getLogPoint(logFile, start, i)))


def main():
    logfile = tempfile.TemporaryFile()
    runTest(logFile=logfile)
    logfile.seek(0)
    measure(logfile)

if __name__ == "__main__":
    main()
