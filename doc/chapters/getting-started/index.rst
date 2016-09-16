
Getting started
***************

Structure
=========

The user API is implemented by the SoftwareContainer Agent as a D-bus service. That API is used
to start, configure, and control containers. On the bus, the Agent has the name
`com.pelagicore.SoftwareContainer` and object path `/com/pelagicore/SoftwareContainerAgent`.

The Agent in turn links to the SoftwareContainer library which is to be considered an internal
API for most users.

The reason for the split between the Agent and the SoftwareContainer library is to allow for
deveopment directly towards the library. A reason for this could be e.g. project specific
requirements on the integration towards SoftwareContainer. The Agent/library split is also useful
when integrating a launcher, that e.g. launches apps from a UI, when the laumcher shouldn't be run
with root privileges. In that case the launcher can use the Agent D-Bus API without being root while
the Agent has been started with root privileges.

Below is an example of the major entities involved in a call to create a container:

.. seqdiag::

    seqdiag {
        Launcher -> Agent [label = "CreateContainer (D-Bus call)", leftnote = "D-Bus side"]
        Agent -> SoftwareContainerLib [label = "init()"];
        SoftwareContainerLib -> ContainerAbstractInterface [label = "initialize()"]
        SoftwareContainerLib <-- ContainerAbstractInterface [label = "ReturnCode"]
        SoftwareContainerLib -> ContainerAbstractInterface [label = "create()"]
        ContainerAbstractInterface -> liblxc [label = "lxc_container_new()", rightnote = "system side"]
        ContainerAbstractInterface <-- liblxc
        SoftwareContainerLib <-- ContainerAbstractInterface [label = "ReturnCode"]
        SoftwareContainerLib -> ContainerAbstractInterface [label = "start()"]
        ContainerAbstractInterface -> liblxc [label = "multiple calls"]
        ContainerAbstractInterface <-- liblxc
        SoftwareContainerLib <-- ContainerAbstractInterface
        Agent <-- SoftwareContainerLib [label = "ReturnCode"]
        Launcher <-- Agent [label = "ID"]
    }


Container config
================

When creating containers, a configuration is passed as a JSON string. The string is an array with objects, in the JSON sense.
The currently supported configs are:

  * Disk writing buffer - on or off. Key: "enableWriteBuffer", value: `true` for on or `false` for off.

Example config JSON::

    [{
        "enableWriteBuffer": true
    }]

The main reason this config is passed as raw JSON is to support additions in supported config options without breaking the API.


Working with containers
=======================

Throughout this section, we will create a container, run an application in it and then shut down the
container again.

Prerequisites:
  * configure and build (see project `README.md`)


From the build directory, run the agent::

    sudo ./agent/softwarecontainer-agent

Note that we are running this command with `sudo`, the Agent needs to be started with root privileges.


Using `dbus-send`, introspect the Agent D-Bus service API::

    dbus-send --system --print-reply \
    --dest=com.pelagicore.SoftwareContainerAgent \
    /com/pelagicore/SoftwareContainerAgent \
    org.freedesktop.DBus.Introspectable.Introspect

Next, we are going to start a new container so take note of the parameters of CreateContainer.


Start a container::

    dbus-send --system --print-reply \
    --dest=com.pelagicore.SoftwareContainerAgent \
    /com/pelagicore/SoftwareContainerAgent \
    com.pelagicore.SoftwareContainerAgent.CreateContainer \
    string:"MyPrefix-" \
    string:'[{"writeOften": "0"}]'

The JSON string passed as argument to the `config` parameter is documented in the Container config section

The return value of CreateContainer is the ID of the newly created container. This is used to identify the container when e.g. shutting it down.


Bind mount a directory inside the container::

    dbus-send --system --print-reply \
    --dest=com.pelagicore.SoftwareContainerAgent \
    /com/pelagicore/SoftwareContainerAgent \
    com.pelagicore.SoftwareContainerAgent.BindMountFolderInContainer \
    uint32:0 \
    string:"/home/vagrant/container-test" \
    string:"app" \
    boolean:false

Parameters:
 * `containerID` - a uint32 with the ID of the created container, as returned by the `CreateConainer` method.
 * `pathInHost` - a string with the host path of the directory to be bind mounted into the container.
 * `subPathInContainer` - a string with the subpath that will be appended to `/gateways` inside the container.
 * `readOnly` - a boolean with a flag to set the bind mounted directory to read only or not. This is currently not supported.

This method assumes the path '/home/vagrant/container-test' exist, but this can be replaced by any other path
if more convenient. The result of the method is that the content of '/home/vagrant/container-test' will be
visible in the path `/gateways/app` inside the container. The actual location on the host can be found in
`/tmp/container/MyPrefix-<generated-name>/gateways/` where the created `app` directory will be.


Launch something in the container::

    dbus-send --system --print-reply \
    --dest=com.pelagicore.SoftwareContainerAgent \
    /com/pelagicore/SoftwareContainerAgent \
    com.pelagicore.SoftwareContainerAgent.LaunchCommand \
    uint32:0 \
    uint32:0 \
    string:"touch hello" \
    string:"/gateways/app/" \
    string:"" dict:string:string:""

Parameters:
 * `containerID` - a uint32 with the ID of the created container, as returned by the `CreateConainer` method.
 * `userID` - currently unused, pass a uint32 '0'.
 * `commandLine` - a string with the method to run at the method line insider the container.
 * `workingDirectory` - a string with a path to a directory which will be set as the working directory.
 * `outputFile` - a string with a path to where stdout will be directed from within the container.
 * `env` - a string:string dictionary with environment variables and values to be set in the container.

The method returns the PID of the process run inside the container.

The above method call results an a file `hello` being created inside the conainer in `/gateways/app/`. This can
also be seen in the bind mounted location `/home/vagrant/container-test/`


Shut down the container::

    dbus-send --system --print-reply \
    --dest=com.pelagicore.SoftwareContainerAgent \
    /com/pelagicore/SoftwareContainerAgent \
    com.pelagicore.SoftwareContainerAgent.ShutDownContainer \
    uint32:0

The value passed as the `containerID` parameter should be the same value that was returned from the call to `CreateContainer`.
