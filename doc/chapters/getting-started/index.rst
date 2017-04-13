
.. _getting-started:

Getting started
***************


Structure
=========

The user API is implemented by the SoftwareContainer Agent as a D-bus service. That API is used
to start, configure, and control containers. On the bus, the Agent has the name
`com.pelagicore.SoftwareContainer` and object path `/com/pelagicore/SoftwareContainerAgent`.

The Agent in turn links to the SoftwareContainer library which is to be considered an internal
API for most users.

The Agent and the SoftwareContainer library are split to allow for development directly towards
the library. One use case where this could be useful is when you have project specific
requirements on the integration towards SoftwareContainer. The Agent/library split is also useful
when integrating a launcher that e.g. launches apps from a UI, when it is not appropriate to
run the launcher with root privileges. In this case the launcher can use the Agent D-Bus API
without being root while the Agent has been started with root privileges.

Below is an example of the major entities involved in a call to create a container:

.. seqdiag::

    span_height = 5;

    Launcher -> Agent [label = "Create (D-Bus call)", leftnote = "D-Bus side"]
    Agent -> SoftwareContainer [label = "init()"];

    SoftwareContainer -> ContainerAbstractInterface [label = "initialize()"]
    SoftwareContainer <-- ContainerAbstractInterface [label = "bool"]

    SoftwareContainer -> ContainerAbstractInterface [label = "create()"]
    ContainerAbstractInterface -> liblxc [label = "lxc_container_new()", rightnote = "system side"]
    ContainerAbstractInterface <-- liblxc
    SoftwareContainer <-- ContainerAbstractInterface [label = "bool"]

    SoftwareContainer -> ContainerAbstractInterface [label = "start()"]
    ContainerAbstractInterface -> liblxc [label = "multiple calls"]
    ContainerAbstractInterface <-- liblxc
    SoftwareContainer <-- ContainerAbstractInterface

    Agent <-- SoftwareContainer [label = "bool"]
    Launcher <-- Agent [label = "ID"]


Container config
================

This section is about the config used when a container is created using the D-Bus interface. This
config is only one pice of the overall configuration, for more info see :ref:`Configuration <configuration>`

When creating containers, a configuration is passed as a JSON string. The string is an array with
objects, in the JSON sense.
The currently supported configs are:

  * Disk writing buffer - on or off. Key: "writeBufferEnabled", value: ``true``
    for on or ``false`` for off.
  * Temporary FileSystem buffers - on or off. Key:
    "temporaryFileSystemWriteBufferEnabled", value ``true`` for on or ``false``
    for off.
  * Temporary FileSystem Size - integer. Key: "temporaryFileSystemSize", value:
    Integer Size in bytes of the Temporary FileSystem Size.

Example config JSON::

    [{
        "writeBufferEnabled": true,
        "temporaryFileSystemWriteBufferEnabled": true,
        "temporaryFileSystemSize": 10485760
    }]

The main reason this config is passed as raw JSON is to support additions in supported config
options without breaking the API.

Write buffer configuration
--------------------------

The ``writeBufferEnabled`` is used to enable or disable write buffers on the mounted
:ref:`filesystems <filesystems>` in the container. These
buffers consists of a RAM overlay on top of the existing :ref:`filesystem <filesystems>`, and are
synced to the underlying :ref:`filesystem <filesystems>` on
shutdown of the container.

The ``temporaryFileSystemWriteBufferEnabled`` is used to enable or disable the
``tmpfs`` being mounted on top of the containers temporary filesystem
containing temporary files. This can be used to separate the containers from
accidentally overcommitting and thereby denying service to any other containers
currently running.

The ``temporaryFileSystemSize`` is used to set the ``RAM size`` of the
``tmpfs`` being mounted on top of the containers temporary filesystem location.
The variable is set in ``bytes``. This will be shared between all the
``upper`` and ``work`` directories being mounted using ``overlayfs`` into a
specific container.


Working with containers
=======================

This section describes how to work with the Agent D-Bus API and how to configure gateways to make
the container useful for specific application and platform needs.


Running a command in a container
--------------------------------

In this section we will create a container, run a command in it and then shut down the
container.

Prerequisites:
  * configure and build (see project `README.md`)


From the build directory, run the agent:

.. literalinclude:: examples/01_start_sc.sh

Note that we are running this command with ``sudo``, the Agent needs to be started with root privileges.

Using e.g. ``gdbus``, we can introspect the Agent D-Bus service API:

.. literalinclude:: examples/02_introspect.sh

Next, we will start a new container so take note of the parameters of Create.


Start a container:

.. literalinclude:: examples/03_create_container.sh

The JSON string passed as argument to the ``config`` parameter is documented in the Container config section.

The return value of Create is the ID of the newly created container. This is used to identify the container when e.g. shutting it down.


Bind mount a directory inside the container:

.. literalinclude:: examples/04_bindmount.sh

Parameters:
 * ``containerID`` - a int32 with the ID of the created container, as returned by the ``Create`` method.
 * ``pathInHost`` - a string with the host path of the directory to be bind mounted into the
   container. The host path must exist before running the command.
 * ``pathInContainer`` - a string representing the absolute mount path inside the container.
 * ``readOnly`` - a boolean with a flag to set the bind mounted directory to read only or not.
   This is currently not supported.

The method assumes the path ``pathInHost`` exists, so choose another path if it is more convenient.
The result of the method is that the content of '/home/vagrant/softwarecontainer' will be
visible in the path ``/app`` inside the container. The actual location on the host can be found in
``/tmp/container/SC-<container ID>/`` where the created ``app`` directory will be.


Launch something in the container:

.. literalinclude:: examples/05_execute.sh

Parameters:
 * ``containerID`` - a int32 with the ID of the created container, as returned by the ``Create`` method.
 * ``commandLine`` - a string with the method to run at the method line insider the container.
 * ``workingDirectory`` - a string with a path to a directory which will be set as the working directory.
 * ``outputFile`` - a string with a path to where stdout will be directed from within the container.
 * ``env`` - a string:string dictionary with environment variables and values to be set in the container. These will override any variables with the same name previously set by the Environment gateway.

The method returns the PID of the process run inside the container.

The above method call results in a file ``hello`` being created inside the conainer in ``/app/``. This can
also be seen in the bind mounted location ``/home/vagrant/softwarecontainer/``.

Suspend the container:

.. literalinclude:: examples/06_suspend.sh

This will suspend execution inside the container. The value passed as the `containerID` parameter
should be the same value that was returned from the call to `Create`. It is not possible
to run LaunchCommand on a suspended container.

Resume the container:

.. literalinclude:: examples/07_resume.sh

This will resume the suspended container. The value passed as the `containerID` parameter
should be the same value that was returned from the call to `Create`.

List all available Capabilities:

.. literalinclude:: examples/08_listcapabilities.sh

This will list all the capabilities that are available and possible to set on
containers.

Set Capabilities on a specific container:

.. literalinclude:: examples/09_setcapabilities.sh

This will set the capabilities listed in the last argument to the container
identified by the `containerID` parameter returned from the `Create` call.

Shut down the container:

.. literalinclude:: examples/10_destroy.sh

The value passed as the `containerID` parameter should be the same value that was returned from the call to `Create`.


Configure gateways
------------------

For details about the gateway configurations, see :ref:`Gateways <gateways>`

Once a container is created and before e.g. an application is launched in the container, gateway
configurations can be set in order to configure what the application will have access to. However,
one does not set gateway configurations directly, they are grouped together into **capabilities**.

Note that this requires you to have service manifests with capabilities defined and pointed out for
the ``softwarecontainer-agent``. See :ref:`service manifests <service-manifests>` for more info.

Set capabilities::

    gdbus call --system \
    --dest com.pelagicore.SoftwareContainerAgent \
    --object-path /com/pelagicore/SoftwareContainerAgent \
    --method com.pelagicore.SoftwareContainerAgent.SetCapabilities \
    0 \
    "['com.acme.example','com.acme.sample']"

Parameters:
 * ``containerID`` - an int with the id of the created container, as returned by the ``Create`` method.
 * ``capabilities`` - a string array of capability names
