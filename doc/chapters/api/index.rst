
.. _api:

API
***

This chapter documents the DBus and C API for interacting with the SoftwareContainerAgent. This is
mostly useful for integrators.

.. TODO::
    This chapter is still to be written properly. Some bits and pieces have
    been but they are under development.

.. _dbus-api:

DBus API
========

:Path: /com/pelagicore/SoftwareContainer
:Interface: com.pelagicore.SoftwareContainerAgent

Methods
-------

Ping
----
Ping()

CreateContainer
---------------
:containerIDPrefix: string
:config: string

uint32 containerID CreateContainer(string containerIDPrefix, string config)

SetContainerName
----------------
:containerID: uint32
:containerName: string

void SetContainerName(uint32 containerID, string containerName)

LaunchCommand
-------------
:containerID: uint32
:userID: uint32
:commandLine: string
:workingDirectory: string
:outputFile: string
:env: map<string, string>

uint32 pid LaunchCommand(uint32 containerID, uint32 userID, string commandLine, string workingDirectory, string outputFile, map<string, string> env)

ShutdownContainer
-----------------
:containerID: uint32

void ShutdownContainer(uint32 containerID)

ShutdownContainerWithTimeout
----------------------------
:containerID: uint32
:timeout: uint32

void ShutdownContainerWithTimeout(uint32 containerID, uint32 timeout)

WriteToStdIn
------------
:processID: uint32
:bytes: array<char>

void WriteToStdIn(uint32 processID, array<char> bytes)

BindMountFolderInContainer
--------------------------
:containerID: uint32
:pathInHost: string
:subPathInContainer: string
:readOnly: bool

string pathInContainer BindMountFolderInContainer(uint32 containerID, string pathInHost, string subPathInContainer, ool readOnly)

SetGatewayConfigs
-----------------
:containerID: uint32
:configs: map<string, string>

void SetGatewayConfigs(uint32 containerID, map<string, string> configs)

SetCapabilities
---------------
:containerID: uint32
:capabilities: array<string>

bool success SetCapabilities(uint32 containerID, array<string> capabilities)

Signals
-------

ProcessStateChanged
-------------------
:containerID: uint32
:processID: uint32
:isRunning: bool
:exitCode: uint32

void ProcessStateChanged(uint32 containerID, uint32 processID, bool isRunning, uint32 exitCode)

C API
-----

