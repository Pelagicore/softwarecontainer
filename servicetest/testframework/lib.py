#
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


class Capability(object):
    """ Represents a capability

        Tests use this to create a capability with associated gateway configs.

        Intention is to allow the configs to be defined as close to the test
        code as possible and avoid the need to have test data files in the
        test tree representing service manifests.

    """

    def __init__(self, name, gw_confs):
        """ name        A string with name of capability.
            gw_confs    A list of dicts with gateway id as key and config as
                        value.
        """
        self.__name = name
        self.__gw_confs = gw_confs
        self.__data = dict()

        self.__create_capability_json()

    def __create_capability_json(self):
        """ Creates a structure like so:

            {
              "name": "some.cap.name",
              "gateways": []
            }

            Where the content of "gateways" will be set by the test. The test
            should also specify the cap name.
        """
        self.__data["name"] = self.__name
        self.__data["gateways"] = list()
        for gw in self.__gw_confs:
            self.__data["gateways"].append(gw)

    def data(self):
        return self.__data


class ServiceManifest(object):
    """ Represents a service manifest which is read by SC at startup

        The 'agent' fixture use this class to create an actual manifest file
        on disk which is used while testing and then cleaned up again.
    """

    def __init__(self, location, name, caps):
        """ location    A string with full path to the directory where the service
                        manifest file will be created.
            name        A string with the service manifest file name
            caps        A list of Capability objects.
        """
        self.__location = location
        self.__name = name
        self.__caps = caps
        self.__manifest_content = dict()

        self.__create_manifest_content()

    def __create_manifest_content(self):
        """ Creates a structure like so:

            {
              "capabilities": []
            }

            Where the content of "capabilities" is defined by the Capability
            objects passed to this object.
        """
        self.__manifest_content["capabilities"] = list()
        for cap in self.__caps:
            self.__manifest_content["capabilities"].append(cap.data())

    def json_as_string(self):
        return json.dumps(self.__manifest_content, indent=4, separators=(",", ": "))

    def name(self):
        return self.__name

    def location(self):
        return self.__location


class StandardManifest(ServiceManifest):
    """ Represents a service manifest that will not be recognised as a default manifest
    """

    def __init__(self, location, name, caps):
        super(StandardManifest, self).__init__(location, name, caps)

    def is_default(self):
        return False


class DefaultManifest(ServiceManifest):
    """ Represents a service manifest that will be recognised as a default manifest
    """

    def __init__(self, location, name, caps):
        super(DefaultManifest, self).__init__(location, name, caps)

    def is_default(self):
        return True


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
    READONLY = False

    def __init__(self):
        bus = dbus.SystemBus()
        pca_obj = bus.get_object("com.pelagicore.SoftwareContainerAgent",
                                 "/com/pelagicore/SoftwareContainerAgent")
        self.__agent = dbus.Interface(pca_obj, "com.pelagicore.SoftwareContainerAgent")
        self.__bind_dir = None
        self.__container_id = None

    def list_capabilities(self):
        """ List all capabilities that can be used for set_capabilities
        """
        caps = self.__agent.ListCapabilities()
        return caps

    def list_containers(self):
        """ List all containers
        """
        containers = self.__agent.List()
        return containers

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
        pid = self.__agent.Execute(self.__container_id,
                                   "{}".format(binary),
                                   self.__bind_dir,
                                   stdout,
                                   env)
        return pid


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
                Container.BIND_MOUNT_DIR: "app",
                Container.READONLY: True
            }

            The values in the dict are passed as arguments to the D-Bus methods on SoftwareContainerAgent.
            The dict is mapped like so:
            Container.CONFIG - argument to SoftwareContainerAgent::Create
            Container.HOST_PATH - second argument to SoftwareContainerAgent::BindMount
            Container.BIND_MOUNT_DIR - third argument to SoftwareContainerAgent::BindMount
            Container.READONLY - fourth argument to SoftwareContainerAgent::BindMount
        """
        container_id = self.__create_container(data[Container.CONFIG])

        self.__bindmount(data[Container.HOST_PATH],
                         data[Container.BIND_MOUNT_DIR],
                         data[Container.READONLY])
        self.__bind_dir = data[Container.BIND_MOUNT_DIR]
        return container_id


    def suspend(self):
        if self.__container_id is not None:
            result = self.__agent.Suspend(self.__container_id)

    def resume(self):
        if self.__container_id is not None:
            result = self.__agent.Resume(self.__container_id)

    def terminate(self):
        """ Perform teardown of container created by call to 'start'
        """
        if self.__container_id is not None:
            result = self.__agent.Destroy(self.__container_id)

    def __create_container(self, config):
        self.__container_id = self.__agent.Create(config)
        return self.__container_id

    def __bindmount(self, host_path, dirname, readonly):
        return self.__agent.BindMount(self.__container_id, host_path, dirname, readonly)


class SoftwareContainerAgentHandler():
    """ Starts the agent and manages its lifecycle, e.g. spawning and killing.

        Used by e.g. ContainerApp over D-Bus.
    """

    def __init__(self, log_file_path=None, caps_dir=None, default_caps_dir=None):
        if log_file_path is None:
            self.__log_file = subprocess.STDOUT
        else:
            self.__log_file = open(log_file_path, "w")

        # Applying arguments to the softwarecontainer-agent call
        cmd = ["softwarecontainer-agent"]
        if caps_dir is not None:
            cmd += ['--manifest-dir', caps_dir]
        if default_caps_dir is not None:
            cmd += ['--default-manifest-dir', default_caps_dir]

        self.__rec = Receiver()
        self.__rec.start()
        self.__rec.wait_until_setup_is_done()

        # Starting softwarecontainer-agent
        # TODO: This doesn't work if the user pass 'None' as log_file_path
        assert log_file_path is not None
        self.__agent = subprocess.Popen(cmd, stdout=self.__log_file, stderr=self.__log_file)

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
