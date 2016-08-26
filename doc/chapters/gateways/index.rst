SoftwareContainer Gateways
**************************

SoftwareContainer provides a set of gateways to enable communication between the host system and the contained system.

Each gateway is dedicated to an IPC mechanism and can then be applied to support multiple services.. This makes the system scalable, as the number of IPC mechanisms is limited while the number of possible services are unlimited.

This chapter contains descriptions of the available gateways and their configuration options.

Gateways
========

CGroups Gateway
---------------

The CGroups Gateway is used to limit the contained system's access to CPU and RAM resources.

Configuration
^^^^^^^^^^^^^

Available in the doxygen docs.



D-Bus Gateway
-------------

The D-Bus Gateway is used to provide access to host system d-bus busses, object paths and interfaces.

Configuration
^^^^^^^^^^^^^

Available in the doxygen docs.



Device Node Gateway
-------------------

The Device Node Gateway is used to provide access to host system device nodes.

Configuration
^^^^^^^^^^^^^

Available in the doxygen docs.



Environment Gateway
-------------------

The Environment Gateway is used to export environment variables into the container.

Configuration
^^^^^^^^^^^^^

Available in the doxygen docs.



File Gateway
------------

The File Gateway is used to export individual files from the host file into the container.

Configuration
^^^^^^^^^^^^^

Available in the doxygen docs.



Network Gateway
---------------

The Network Gateway is used to provide Internet access through a container-specific firewall.

Configuration
^^^^^^^^^^^^^

Available in the doxygen docs.



PulseAudio Gateway
------------------

The PulseAudio Gateway is used to provide access to the host system PulseAudio server.

Configuration
^^^^^^^^^^^^^

Available in the doxygen docs.



Wayland Gateway
---------------

The Wayland Gateway is used to provide access to the hsot system Wayland server.

Configuration
^^^^^^^^^^^^^

Available in the doxygen docs.
