.. _integration-guidelines:

Integration guidelines
**********************

This chapter describes the workflow, major tasks and concepts involved in integrating
SoftwareContainer into a platform. A brief summary of the general steps involved:

#. Setting up the various IPC mechanisms, that should be reachable from inside containers, on the
   host system.
#. Writing service manifests for capabilities of the platform.
#. Integrating a launcher that interact with SoftwareContainer.

Note that SoftwareContainer as a project is split into multiple sub-components, and while it is
possible to use e.g. the container library directly, these guidelines assumes the integration point
is the complete component.

Below follows information about what SoftwareContainer assumes about the host system when handling
different IPC mechanisms and examples of configuration and usage.

.. include:: 10-service-manifests.rst

.. _network-setup:
.. include:: 10-network-setup.rst

.. include:: 10-wayland-setup.rst
.. include:: 10-the-launcher.rst
.. include:: 10-traffic-shaping.rst
.. include:: 10-shared-memory.rst
.. include:: 10-write-buffers.rst
.. include:: 10-systemd-service.rst
.. include:: 10-resource-management.rst
