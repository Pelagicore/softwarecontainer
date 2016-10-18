.. _api:

API
***

This chapter documents the DBus and C API for interacting with the SoftwareContainerAgent. This is
mostly useful for integrators.

.. _dbus-api:

D-Bus API
========

D-Bus API is an IPC interface to call SoftwareContainer agent methods.

:Path: /com/pelagicore/SoftwareContainer
:Interface: com.pelagicore.SoftwareContainerAgent

Methods
-------

Ping
----
Controls availability of com.pelagicore.SoftwareContainerAgent interface

:Parameters:
        *None*
|
:Return Value:
        *None*
|
:Example Usage:

::

        dbus-send --system --print-reply \
        --dest=com.pelagicore.SoftwareContainerAgent \
        /com/pelagicore/SoftwareContainerAgent \
        com.pelagicore.SoftwareContainerAgent.Ping

CreateContainer
---------------
Creates a new container and returns created container id.

:Parameters:
        :IDPrefix: ``string`` prefix for random container id.
        :config: ``string`` config file in json format.
        
                Example config JSON::
        
                [{"enableWriteBuffer": true}]
|
:Return Value:
        :containerID: ``uint32`` ID of created SoftwareContainer.
|
:Example Usage:

::

        dbus-send --system --print-reply \
        --dest=com.pelagicore.SoftwareContainerAgent \
        /com/pelagicore/SoftwareContainerAgent \
        com.pelagicore.SoftwareContainerAgent.CreateContainer \
        string:"00" string:'[{"enableWriteBuffer": true}]'

SetContainerName
----------------
Sets the name of container with unique containerID.

:Parameters:
        :containerID: ``uint32`` The ID obtained by CreateContainer method.
        :containerName: ``string`` name.
|
:Return Value:
        *None*
|
:Example Usage:

::

        dbus-send --system --print-reply \
        --dest=com.pelagicore.SoftwareContainerAgent \
        /com/pelagicore/SoftwareContainerAgent \
        com.pelagicore.SoftwareContainerAgent.SetContainerName \
        uint32:0 string:"myContainer"


LaunchCommand
-------------
Launches specific application/code int he container.

:Parameters:
        :containerID: ``uint32`` The ID obtained by CreateContainer method.
        :userID: ``uint32`` UID for command, currently unused, use ``0``.
        :commandLine: ``string`` the method to run in container.
        :workDirectory: ``string`` path to working directory.
        :outputFile: ``string`` output file to direct stdout.
        :env: ``map<string, string>`` environment variables and their values.
|
:Return Value:
        :pid: ``uint32`` PID of the process run inside the container.
|
:Example Usage:

::

        dbus-send --system --print-reply \
        --dest=com.pelagicore.SoftwareContainerAgent \
        /com/pelagicore/SoftwareContainerAgent \
        com.pelagicore.SoftwareContainerAgent.LaunchCommand \
        uint32:0 \
        uint32:0 \
        string:"touch hello" \
        string:"/gateways/app/" \
        string:"" \
        dict:string:string:""
        

ShutdownContainer
-----------------
Teardowns all active gateways related to container and shutdowns the container with all reserved sources.

:Parameters:
        :containerID: ``uint32`` The ID obtained by CreateContainer method.
|
:Return Value:
        *None*
|
:Example Usage:

::

        dbus-send --system --print-reply \
        --dest=com.pelagicore.SoftwareContainerAgent \
        /com/pelagicore/SoftwareContainerAgent \
        com.pelagicore.SoftwareContainerAgent.ShutDownContainer \
        uint32:0


ShutdownContainerWithTimeout
----------------------------
Teardowns all active gateways related to container and shutdowns the container and all reserved sources after given timeout.

:Parameters:
        :containerID: ``uint32`` The ID obtained by CreateContainer method.
        :timeout: ``uint32`` timeout. 
|
:Return Value:
        *None*
|
:Example Usage:

::

        dbus-send --system --print-reply \
        --dest=com.pelagicore.SoftwareContainerAgent \
        /com/pelagicore/SoftwareContainerAgent \
        com.pelagicore.SoftwareContainerAgent.ShutDownContainerWithTimeout \
        uint32:0 \
        uint32:5


WriteToStdIn
------------
Send a character array to the standard input of a particular process.

:Parameters:
        :processID: ``uint32`` PID of the process; obtained by LaunchCommand.
        :bytes: ``array<char>`` character array to sent to the stdin.
|
:Return Value:
        *None*
|
:Example Usage:

::

        bus-send --system --print-reply \
        --dest=com.pelagicore.SoftwareContainerAgent \
        /com/pelagicore/SoftwareContainerAgent \
        com.pelagicore.SoftwareContainerAgent.WriteToStdIn \
        uint32:14859 \
        array:byte:'a','b'        


BindMountFolderInContainer
--------------------------
Binds a directory on the host to the container.

:Parameters:
        :containerID: ``uint32`` The ID obtained by CreateContainer method.
        :pathInHost: ``string`` path to the directory in host.
        :subPathInContainer: ``string`` path to the directory in container.
        :readOnly: ``bool`` indicates whether the directory is read-only or not.  
|
:Return Value:
        :pathInContainer: ``string`` path to the bind folder in container. 
|
:Example Usage:

::

        dbus-send --system --print-reply \
        --dest=com.pelagicore.SoftwareContainerAgent \
        /com/pelagicore/SoftwareContainerAgent \
        com.pelagicore.SoftwareContainerAgent.BindMountFolderInContainer \
        uint32:1 \
        string:"/home/myUser/myBindFolder" \
        string:"/home/vagrant/softwarecontainer/build" \
        boolean:false
        

SetGatewayConfigs
-----------------
Sets the configuration of a particular gateway. The gateway configuration contains settings as key/value pairs.

:Parameters:
        :containerID: ``uint32`` The ID obtained by CreateContainer method.
        :configs: ``map<string, string>`` A map to key/value pairs.
|
:Return Value:
        *None*
| 
:Example Usage:

::

        dbus-send --system --print-reply \
        --dest=com.pelagicore.SoftwareContainerAgent \
        /com/pelagicore/SoftwareContainerAgent \
        com.pelagicore.SoftwareContainerAgent.SetGatewayConfigs \
        uint32:1 \
        dict:string:string:"configItem1","1","configItem2","2"


SetCapabilities
---------------
Currently This method has no applicable usage. 

:Parameters:
        :containerID: ``uint32`` The ID obtained by CreateContainer method.
        :capabilities: ``array<string>``
|
:Return Value:
        :success: ``boolean`` either true or false.
|
:Example Usage:

::

         dbus-send --system --print-reply \
         --dest=com.pelagicore.SoftwareContainerAgent \
         /com/pelagicore/SoftwareContainerAgent \
         com.pelagicore.SoftwareContainerAgent.SetCapabilities \
         uint32:1 \
         array:string:"I","Can","not","provide","functionality"


Signals
-------

ProcessStateChanged
-------------------
The D-Bus API sends signal when process state is changed. There are four value to be emitted.

:containerID: ``uint32`` The ID obtained by CreateContainer method.

:processID: ``uint32`` Pocess ID of container.

:isRunning: ``bool`` Whether the process is running or not.

:exitCode: ``uint32`` exit code of Process.


Introspection
-------------

Using ``org.freedesktop.DBus.Introspectable.Introspect`` interface, methods of SoftwareContainerAgent D-Bus API can be observed.

:Example Usage:

::

        dbus-send --system --print-reply \
        --dest=com.pelagicore.SoftwareContainerAgent \
        /com/pelagicore/SoftwareContainerAgent \
        org.freedesktop.DBus.Introspectable.Introspect

