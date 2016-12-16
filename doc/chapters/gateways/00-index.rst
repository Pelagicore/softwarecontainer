
.. _gateways:

Gateways
********

SoftwareContainer provides a set of gateways to enable communication between the host system and the
contained system.

Each gateway is dedicated to an IPC mechanism and can then be applied to support multiple services.
This makes the system scalable, as the number of IPC mechanisms is limited while the number of
possible services are unlimited.  *Currently, some gateways break this principle, which is subject
to correction in the future. The design intention is to have the gateways IPC centric and construct
more abstract concepts like a Wayland "gateway" on top of multiple IPC gateways.*

This chapter contains descriptions of the available gateways, their IDs,  and their configuration
options.  It is intended to explain to e.g. a service integrator how to write gateway configurations
suitable for the service.

For more information how to integrate and use the configurations in a project,
see :ref:`Integration guidelines <integration-guidelines>`.

For more information how to develop and integrate gateways in SoftwareContainer,
see :ref:`Developer guidelines <developers>`.


Note on configurations
======================

All gateway configurations are JSON arrays containing valid JSON elements, and SoftwareContainer
requires this.  Beyond that, the structure and content of this JSON is the responsibility of the
respective gateway.

.. include:: 10-cgroups-gateway.rst
.. include:: 10-device-node-gateway.rst
.. include:: 10-dbus-gateway.rst
.. include:: 10-environment-gateway.rst
.. include:: 10-file-gateway.rst
.. include:: 10-network-gateway.rst
.. include:: 10-pulse-gateway.rst
.. include:: 10-wayland-gateway.rst
