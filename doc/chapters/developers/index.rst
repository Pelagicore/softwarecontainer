
.. _developers:

Developer guidelines
********************

This section describes the internal structure of SoftwareContainer, and the major design considerations.

The intended reader is a developer who wants to understand and work with the internals of SoftwareContainer,
or anyone who simply wants a better insight for e.g. troubleshooting.

There is also some documentation of interest in the :ref:`Filesystems <filesystems>` chapter
regarding filesystems in the container.

.. todo::
    The diagrams in this chapter do not differ between sync and async calls, should be fixed


Important internal interfaces
=============================

Interface: softwarecontainer.h
---------------------------------

The Agent uses ``softwarecontainer.h`` to manage the lifecycle of container instances, e.g. creation and
destruction. The interface provides access to the underlying ``ContainerAbstractInterface`` which users can
utilize to access the container interface. The interface is implemented by ``SoftwareContainer``.

Below is a diagram showing the interaction between the user `Agent` and the interfaces ``SoftwareContainer``
subsequently uses:

.. seqdiag::

    span_height = 5;


    Agent -> SoftwareContainer [label = "new()"]
    Agent <-- SoftwareContainer

    Agent -> SoftwareContainer [label = "setContainerIDPrefix()"]
    Agent <-- SoftwareContainer

    Agent -> SoftwareContainer [label = "setMainLoopContext()"]
    Agent <-- SoftwareContainer

    Agent -> SoftwareContainer [label = "init()"]

    SoftwareContainer -> SoftwareContainer [label = "preload()"]

    SoftwareContainer -> ContainerAbstractInterface [label = "initialize()"]
    SoftwareContainer <-- ContainerAbstractInterface [label = "ReturnCode"]

    SoftwareContainer -> ContainerAbstractInterface [label = "create()"]
    SoftwareContainer <-- ContainerAbstractInterface [label = "ReturnCode"]

    SoftwareContainer -> ContainerAbstractInterface [label = "start()"]
    SoftwareContainer <-- ContainerAbstractInterface [label = "ReturnCode"]

    SoftwareContainer -> Gateway [label = "new()", note = "multiple calls"]
    SoftwareContainer <-- Gateway

    SoftwareContainer -> SoftwareContainer [label = "addGateway()", note = "multiple calls"]

    SoftwareContainer -> Gateway [label = "setContainer()", note = "multiple calls"]
    SoftwareContainer <-- Gateway

    Agent <-- SoftwareContainer [label = "ReturnCode"]

    === further events here ===


Interface: containerabstractinterface.h
---------------------------------------

The abstract class ``ContainerAbstractInterface`` is the abstraction of the underlying container technology.
The design intention is to have this interface container technology agnostic. The design is intended to:

* Separate the higher level APIs and interfaces from the details of the container implementation.
* Allow easier integration of other container technologies.
* Isolate container implementation and domain specific details to enter the higher level code.

This is currently implemented by the LXC specific ``Container`` in `container.h`.

Below is a diagram showing the `initialize`, `create`, and `start` sequence focussing on the
``ContainerAbstractInterface`` implementation ``Container``:

.. seqdiag::

    span_height = 5;

    SoftwareContainer -> Container [label = "initialize()"]
    Container -> Container [label = "createDirectory()"]
    Container -> Container [label = "createSharedMountPoint()"]
    SoftwareContainer <-- Container [label = "ReturnCode"]

    SoftwareContainer -> Container [label = "create()"]
    Container -> liblxc [label = "lxc_container_new()"]
    Container <-- liblxc [label = "container_struct"]
    === various operations on the lxc struct ===
    SoftwareContainer <-- Container [label = "ReturnCode"]

    SoftwareContainer -> Container [label = "start()"]
    === various operations on the lxc struct ===
    SoftwareContainer <-- Container [label = "ReturnCode"]


Interface: gateway.h
--------------------

All gateway implementations must inherit ``Gateway`` and implement the pure virtual methods. The
rationale and design intention for isolating gateway specific knowledge to respective gateway is to:

* Allow gateways to have a flexible config structure and content to more easily suit their purpose.
* Separate maintenance between gateways, e.g. updating the config and implementation of one will not
  propagate to the others.
* Consistent interface towards the user of the class so there are no ripple effects into SoftwareContainer.

SoftwareContainer sets the configuration on the interface of this base class, and the derived classes are then called
internally to do their specific parsing and application of the configs.

SoftwareContainer also queries the gateways about state and e.g. activates the gateway when it has been configured.

Below diagram show the major events during the configuration and activation sequence initiated by the Agent:

.. seqdiag::

    span_height = 5;


    Agent -> SoftwareContainer [label = "updateGatewayConfiguration()"]
    SoftwareContainer -> SoftwareContainer [label = "setGatewayConfigs()"]

    SoftwareContainer -> Gateway [label = "id()"]
    SoftwareContainer <-- Gateway [label = "ID"]

    SoftwareContainer -> Gateway [label = "setConfig()"]

    Gateway -> derived-gateway [label = "readConfigElement()"]
    Gateway <-- derived-gateway [label = "bool"]

    SoftwareContainer <-- Gateway [label = "bool"]

    SoftwareContainer -> Gateway [label = "isConfigured()"]
    SoftwareContainer <-- Gateway [label = "bool"]

    SoftwareContainer -> Gateway [label = "activate()"]
    SoftwareContainer <-- Gateway [label = "bool"]

    Agent <-- SoftwareContainer [label = "void", failed]
