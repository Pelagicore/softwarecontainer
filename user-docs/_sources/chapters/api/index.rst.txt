.. _api:

API
***

This chapter documents the D-Bus API for interacting with the SoftwareContainerAgent. This is mostly
useful for integrators, or anyone writing a launcher for running apps inside SoftwareContainer
containers.

.. _dbus-api:

D-Bus API
=========

The D-Bus API is an interface to call SoftwareContainerAgent methods. The API provides the following
object path and interface.

* Object Path: /com/pelagicore/SoftwareContainerAgent
* Interface: com.pelagicore.SoftwareContainerAgent

Prerequisities
--------------
The prerequisities for each method describe what state the SoftwareContainer system needs to be in
for a call to that method to be valid. For most methods, the only prerequisite is a successfully
created container, which is the same as getting a container ID returned from the Create method.

Error Handling
--------------
All methods in this API can and will return D-Bus errors on failure. All calls that don't return a
D-Bus error have worked successfully. Any call that returns a D-Bus error has failed. All calls that
change a state and raise a D-Bus error shall have their side effects documented in this chapter.

For each method, we describe the various error types and their potential sources. Some of these
errors have a note saying that they put the container in an `invalid state`. An invalid state means
that no other operations can be reliably done, and SoftwareContainer will disallow any operations on
a container that is in an invalid state, except for destroying them.

Methods
-------

BindMount
~~~~~~~~~
Binds a path on the host to the container. Any missing parent directories in the path will be
created.

Parameters
##########
* containerID: ``int32`` The ID obtained by Create method.
* pathInHost: ``string`` absolute path in the host.
* pathInContainer: ``string`` the absolute path to mount to in the container.
* readOnly: ``bool`` indicates whether the directory is read-only or not.

Prerequisities
##############
* A successful call to Create, such that it returned a container ID.
* The container is not suspended

Error sources
#############
* Invalid ID: No matching container exists.
* Bad state: The container is not started or ready
* Bad path
    * The host path does not exist
    * The destination path is not absolute
    * The destination path is already mounted to
* Filesystem issues
    * Could not create parent directories
    * Could not create the target directory (if mounting a directory)
    * Could not touch the target file (if mounting a file)
* Mount issues
    * Mount failed (:linuxman:`mount(2)` failed)
    * Couldn't move the mount
    * Failed to re-mount read-only

**Note:** Currently, failing mounts are not rolled back.

Create
~~~~~~
Creates a container with given configuration.

Parameters
##########
* config: ``string`` config file in json format.

Example config JSON::

[{"writeBufferEnabled": true}]

Return Values
#############
* containerID: ``int32`` ID of created SoftwareContainer.

Prerequisities
##############
None

Error sources
#############
* Network setup issues
    * Bridge device not available and unable to set it up
    * Given netmask can not be used to assign IP address to container
    * See :ref:`Network setup<network-setup>`

* LXC issues
    * Couldn't prepare file system while initializing container
    * Container ID already being in use (caused by lingering old containers)
    * LXC config couldn't be loaded
    * LXC could not create container

If the error about ID already being in use shows up, one can try to restart the
softwarecontainer-agent, since it does a cleanup of old containers on startup.

Destroy
~~~~~~~
Tears down all active gateways related to container and shuts down the container with all reserved
sources.

Parameters
##########
* containerID: ``int32`` The ID obtained by Create method.

Prerequisities
##############
* A successful call to Create, such that it returned a container ID.

Error sources
#############
* Invalid ID: No matching container exists.
* Invalid state: The container is not ready or is suspended
* Gateway errors: One or more of the gateways might have failed to shut down
* LXC issues:
    * LXC could not shutdown the container
    * LXC could not force stop the container (if shutdown fails)
    * LXC could not destroy the container

Failing to destroy the container leads to it being put into an invalid state. Since destroying an
invalid container is what one normally does, this is a difficult error to handle. We recommend
shutting down the SoftwareContainerAgent, since it does some cleanup of old containers on startup.
If that does not work, one can try to use the LXC userspace tools `lxc-stop` and `lxc-destroy`.

.. _dbus-execute:

Execute
~~~~~~~
Launches the specified application/code in the container.

Parameters
##########
* containerID: ``int32`` The ID obtained by Create method.
* commandLine: ``string`` the method to run in container.
* workDirectory: ``string`` path to working directory.
* outputFile: ``string`` output file to direct stdout.
* env: ``map<string, string>`` environment variables and their values.

Return value
############
* pid: ``int32`` PID of process running in the container, as seen by the host.

Prerequisities
##############
* A successful call to Create, such that it returned a container ID.
* The container is not suspended
* The workDirectory path has to exist inside the container.

Error sources
#############
* Invalid ID: No matching container exists.
* Bad state: The container is not ready or is suspended
* Capability error: couldn't set default capabilities (if ``SetCapabilities`` has not already been
  called)
* LXC error: The underlying LXC method call failed.

**Note:** Even if the ``Execute`` call works fine, that doesn't mean the command that is being run
inside the container runs fine. For example, it is possible to pass command-lines that point to
non-executables, or non-existing files. One would notice this however, by getting a
``ProcessStateChanged`` signal sent when the call exits.

List
~~~~
Returns a list of the current containers

Return value
############
* containers: ``array<int32>`` IDs for all containers

Prerequisities
##############
None

Error sources
#############
None, this method only inspects the current state

ListCapabilities
~~~~~~~~~~~~~~~~
Lists all capabilities that the user can apply. Note that this does not include the default
capabilities, which are not listable.

Return value
############
* capabilities: ``array<string>`` all available capability names

Prerequisities
##############
None

Error sources
#############
None, this method only inspects the current state

Resume
~~~~~~
Resumes a suspended container

Parameters
##########
* containerID: ``int32`` The ID obtained by Create method.

Prerequisities
##############
* A successful call to Create, such that it returned a container ID.
* The container is suspended

Error sources
#############
* Invalid ID: No matching container exists.
* Bad state: The container is not suspended
* LXC error: Couldn't resume the container

**Note:** Failure to resume a container leads to it being put in an invalid state.

SetCapabilities
~~~~~~~~~~~~~~~
Applies the given list of capability names to the container. Capabilities are mapped to gateway
configurations and applied to each gateway for which they map a configuration.

Parameters
##########
* containerID: ``int32`` The ID obtained by Create method.
* capabilities: ``array<string>`` of capability names

Prerequisities
##############
* A successful call to Create, such that it returned a container ID.
* That the container is not suspended

Error sources
#############
* Invalid ID: No matching container exists.
* Bad state: The container is not ready
* The given array of capabilities is empty (semantically not an error though)
* Bad capabilities: The capabilities (including any default capabilities) failed to apply
    * Syntax error: The gateway configs in the capability could be missing keys or be malformed.
    * Trying to use an unknown gateway ID
    * Gateway error: A gateway failed to apply a specific configuration

Suspend
~~~~~~~
Suspends all execution inside a given container.

Parameters
##########
* containerID: ``int32`` The ID obtained by Create method.

Prerequisities
##############
* A successful call to Create, such that it returned a container ID.
* That the container is not suspended

Error sources
#############
* Invalid ID: No matching container exists.
* Bad state: The container is not ready, or is already suspended
* LXC error: Couldn't suspend the container

**Note:**: Failing to suspend the container, other than it being in a bad state, leads to it being
put in an invalid state.

Signals
-------

ProcessStateChanged
~~~~~~~~~~~~~~~~~~~
The D-Bus API sends signal when process state is changed. There are four values to be emitted.

Parameters
##########
* containerID: ``int32`` The ID obtained by Create method.
* processID: ``uint32`` Pocess ID of container.
* isRunning: ``bool`` Whether the process is running or not.
* exitCode: ``uint32`` exit code of Process.

Introspection
-------------

Using the ``org.freedesktop.DBus.Introspectable.Introspect`` interface, methods of the
SoftwareContainerAgent D-Bus API can be observed.
