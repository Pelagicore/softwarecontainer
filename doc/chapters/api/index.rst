.. _api:

API
***

This chapter documents the D-Bus API for interacting with the
SoftwareContainerAgent. This is mostly useful for integrators.

.. _dbus-api:

D-Bus API
=========

D-Bus API is an IPC interface to call SoftwareContainer agent methods. The API
provides following object path and interface.

:Object Path: /com/pelagicore/SoftwareContainer
:Interface: com.pelagicore.SoftwareContainerAgent

Methods
-------
All methods that modify the state of the containers return a ``success``
variable along with the actual return value. This is to not have to rely on
magic values. The two methods ``List`` and ``ListCapabilities`` are the only
ones that doesn't modify the state.

BindMount
---------
Binds a directory on the host to the container.

:Parameters:
        :containerID: ``int32`` The ID obtained by CreateContainer method.
        :pathInHost: ``string`` absolute path to the directory in the host.
        :pathInContainer: ``string`` the absolute path to the directory in container.
        :readOnly: ``bool`` indicates whether the directory is read-only or not.

|

:Return Value:
        :success: ``bool`` Whether or not the operation was successful.

Create
------
Creates a container with given configuration.

:Parameters:
        :config: ``string`` config file in json format.

                Example config JSON::

                [{"enableWriteBuffer": true}]

|

:Return Values:
        :containerID: ``int32`` ID of created SoftwareContainer.
        :success: ``bool`` Whether or not the operation was successful.

Destroy
-------
Tears down all active gateways related to container and shuts down the
container with all reserved sources.

:Parameters:
        :containerID: ``int32`` The ID obtained by CreateContainer method.

|

:Return Value:
        :success: ``bool`` Whether or not the operation was successful.

Execute
-------
Launches the specified application/code in the container.

:Parameters:
        :containerID: ``int32`` The ID obtained by CreateContainer method.
        :commandLine: ``string`` the method to run in container.
        :workDirectory: ``string`` path to working directory.
        :outputFile: ``string`` output file to direct stdout.
        :env: ``map<string, string>`` environment variables and their values.

|

:Return Values:
        :pid: ``int32`` PID of the process run inside the container.
        :success: ``bool`` Whether or not the operation was successful.

List
----
Returns a list of the current containers

:Return Value:
        :containers: ``array<int32>`` IDs for all containers

ListCapabilities
----------------
Lists all capabilities that the user can apply. Note that this does not include
the default capabilities, which are not listable.

:Return Value:
        :capabilities: ``array<string>`` all available capability names

Resume
------
Resumes a suspended container

:Parameters:
        :containerID: ``int32`` The ID obtained by CreateContainer method.

|

:Return Value:
        :success: ``bool`` Whether or not the operation was successful.

SetCapabilities
---------------
Applies the given list of capability names to the container. Capabilities are
mapped to gateway configurations and applied to each gateway for which they
map a configuration.

:Parameters:
        :containerID: ``int32`` The ID obtained by CreateContainer method.
        :capabilities: ``array<string>`` of capability names

|

:Return Value:
        :success: ``bool`` Whether or not the operation was successful.

Suspend
-------
Suspends all execution inside a given container.

:Parameters:
        :containerID: ``int32`` The ID obtained by CreateContainer method.

|

:Return Value:
        :success: ``bool`` Whether or not the operation was successful.

SetCapabilities
---------------
Applies the given list of capability names to the container. Capabilities are mapped to gateway
configurations and applied to each gateway for which they map a configuration.

:Parameters:
        :containerID: ``int32`` The ID obtained by CreateContainer method.

|

:Return Value:
        :success: ``bool`` Whether or not the operation was successful.

Signals
-------

ProcessStateChanged
-------------------
The D-Bus API sends signal when process state is changed. There are four values to be emitted.

:containerID: ``int32`` The ID obtained by CreateContainer method.

:processID: ``uint32`` Pocess ID of container.

:isRunning: ``bool`` Whether the process is running or not.

:exitCode: ``uint32`` exit code of Process.


Introspection
-------------

Using ``org.freedesktop.DBus.Introspectable.Introspect`` interface, methods of
SoftwareContainerAgent D-Bus API can be observed.


