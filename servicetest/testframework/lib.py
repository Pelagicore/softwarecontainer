
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

    For example, the Container class is probably something the tests want to
    use directly, while the Agent setup is provided more handlily by fixtures.
"""

import dbus
import json
import threading
import thread
import Queue
import subprocess
import dbus.mainloop.glib
from gi.repository import GObject


class Container():
    """ This represents a container for most purposes.

        This can be considered a proxy for the Agent, but the main purpose is to have a way
        to create, manipulate, interact, and destroy a container instance.

        TODO: The name of this class is off. What is it really? Resembles the "convenience" API
              which has been discussed.
    """

    # Static class members used by tests for container "data" dict keys. See the documentation
    # for the start() method for details.
    CONFIG = "config"
    BIND_MOUNT_DIR = "bind-mount-dir"
    HOST_PATH = "host-path"

    def __init__(self):
        bus = dbus.SystemBus()
        pca_obj = bus.get_object("com.pelagicore.SoftwareContainerAgent",
                                 "/com/pelagicore/SoftwareContainerAgent")
        self.__agent = dbus.Interface(pca_obj, "com.pelagicore.SoftwareContainerAgent")
        self.__bind_dir = None
        self.__container_id = None

    def set_gateway_config(self, gateway_id, config):
        """ Set a gateway config by passing an id and a Python object equivalent to a JSON
            config.
        """
        self.__agent.SetGatewayConfigs(self.__container_id, {gateway_id: json.dumps(config)})

    def set_capabilities(self, capabilities):
        """ Set capabilities by passsing a list of strings with capability IDs
        """
        result = self.__agent.SetCapabilities(self.__container_id, capabilities)
        return True if result == dbus.Boolean(True) else False

    def launch_command(self, binary, stdout="/tmp/stdout", env={"": ""}):
        """ Calls LaunchCommand on the Agent D-Bus interface.

            The user must pass the actual command to run. This is passed as a string argument
            to the 'binary' parameter and is passed along as is to the D-Bus method. stdout and
            environment dictionary are optional. The other arguments required by the D-Bus method
            are set by this helper based on other configs and data passed from the user previously.
        """
        response = self.__agent.LaunchCommand(self.__container_id,
                                              0,
                                              "{}".format(binary),
                                              self.__bind_dir,
                                              stdout,
                                              env)
        if response is -1:
            print "Failed to launch process in container"
            return -1
        return response

    def get_bind_dir(self):
        """ Returns the path containing the bind mounted dir set previously

            Tests can call this method to know what path prefix is suitable when running
            a binary which has been bind mounted into the container previously.
        """
        return self.__bind_dir

    def start(self, data):
        """ Creates a container and bind mounts a directory.

            The data parameter takes a dictionary with various values and configs to be used by
            this helper class. The definitions for the dict keys are the class members defined at
            the beginning of this class.

            The user should pass a dict like e.g.:
            {
                Container.CONFIG: "{enableWriteBuffer: false}",
                Container.HOST_PATH: my_path_string_variable,
                Container.BIND_MOUNT_DIR: "app"
            }

            The values in the dict are passed as arguments to the D-Bus methods on SoftwareContainerAgent.
            The dict is mapped like so:
            Container.CONFIG - argument to SoftwareContainerAgent::CreateContainer
            Container.HOST_PATH - second argument to SoftwareContainerAgent::BindMountFolderInContainer
            Container.BIND_MOUNT_DIR - third argument to SoftwareContainerAgent::BindMountFolderInContainer
        """
        self.__create_container(data[Container.CONFIG])
        self.__bind_dir = self.__bindmount_folder_in_container(data[Container.HOST_PATH],
                                                               data[Container.BIND_MOUNT_DIR])

    def terminate(self):
        """ Perform teardown of container created by call to 'start'
        """
        if self.__container_id is not None:
            self.__agent.ShutDownContainer(self.__container_id)

    def __create_container(self, config):
        self.__container_id = self.__agent.CreateContainer(config)

    def __bindmount_folder_in_container(self, host_path, dirname):
        return self.__agent.BindMountFolderInContainer(self.__container_id, host_path, dirname, True)


class SoftwareContainerAgentHandler():
    """ Starts the agent and manages its lifecycle, e.g. spawning and killing.

        Used by e.g. ContainerApp over D-Bus.
    """

    def __init__(self, log_file_path=None):
        self.__log_file = None
        if log_file_path is None:
            self.__log_file = subprocess.STDOUT
        else:
            self.__log_file = open(log_file_path, "w")
        self.__rec = Receiver()
        self.__rec.start()
        self.__rec.wait_until_setup_is_done()

        # Starting softwarecontainer-agent
        # TODO: This doesn't work if the user pass 'None' as log_file_path
        assert log_file_path is not None
        self.__agent = subprocess.Popen("softwarecontainer-agent", stdout=self.__log_file, stderr=self.__log_file)

        try:
            # Wait for the softwarecontainerStarted message to appear on the
            # msgQueue, this is evoked when softwarecontainer-agent is ready to
            # perform work. If we timeout tear down what we have started so far.
            while self.__rec.msg_queue().get(block=True, timeout=5) != "softwarecontainerStarted":
                pass
        except Queue.Empty as e:
            self.__agent.terminate()
            self.__rec.terminate()
            raise Exception("SoftwareContainer DBus interface not seen", e)

        if self.__agent.poll() is not None:
            # Make sure we are not trying to perform anything against a dead softwarecontainer-agent
            self.__rec.terminate()
            raise Exception("SoftwareContainer-agent has died for some reason")

    def terminate(self):
        self.__agent.terminate()
        self.__rec.terminate()
        self.__log_file.close()


class Receiver(threading.Thread):
    """ The Receiver class encapsulates and runs a gobject mainloop and dbus implementation in a separate thread

        Purpose of Receiver is to listen to NameOwnerChanged to know about Agent coming and going
    """
    def __init__(self):
        threading.Thread.__init__(self)
        GObject.threads_init()
        self.__msg_queue = Queue.Queue()
        self.lock = thread.allocate_lock()
        self.lock.acquire()

    def wait_until_setup_is_done(self):
        self.lock.acquire()
        self.lock.release()

    def msg_queue(self):
        return self.__msg_queue

    def handler(self, object_path, _name, _old_owner):
        if object_path == "com.pelagicore.SoftwareContainerAgent":
            # Put softwarecontainerStarted on the message queue, this is picked up by other threads which
            # should continue running when softwarecontainer-agent is ready to work.
            self.__msg_queue.put("softwarecontainerStarted")

    def run(self):
        self.__gloop = GObject.MainLoop()
        self.__loop = dbus.mainloop.glib.DBusGMainLoop()
        self.__bus = dbus.SystemBus(mainloop=self.__loop)
        self.__bus.add_signal_receiver(self.handler,
                                       dbus_interface="org.freedesktop.DBus",
                                       signal_name="NameOwnerChanged")
        self.lock.release()
        self.__gloop.run()

    def terminate(self):
        if self.__loop is not None:
            self.__gloop.quit()
