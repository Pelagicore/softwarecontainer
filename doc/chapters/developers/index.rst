
.. _developers:

Developer guidelines
********************

This section describes the internal structure of SoftwareContainer, and the major design considerations.

The intended reader is a developer who wants to understand and work with the internals of SoftwareContainer,
or anyone who simply wants a better insight for e.g. troubleshooting.

.. todo::
    The diagrams in this chapter do not differ between sync and async calls, should be fixed


Important internal interfaces
=============================

Interface: libsoftwarecontainer.h
---------------------------------

The Agent uses ``libsoftwarecontainer.h`` to manage the lifecycle of container instances, e.g. creation and
destruction. The interface provides access to the underlying ``ContainerAbstractInterface`` which users can
utilize to access the container interface. The interface is implemented by ``SoftwareContainerLib``.

Below is a diagram showing the interaction between the user `Agent` and the interfaces ``SoftwareContainerLib``
subsequently uses:

.. seqdiag::

    span_height = 5;


    Agent -> SoftwareContainerLib [label = "new()"]
    Agent <-- SoftwareContainerLib

    Agent -> SoftwareContainerLib [label = "setContainerIDPrefix()"]
    Agent <-- SoftwareContainerLib

    Agent -> SoftwareContainerLib [label = "setMainLoopContext()"]
    Agent <-- SoftwareContainerLib

    Agent -> SoftwareContainerLib [label = "init()"]

    SoftwareContainerLib -> SoftwareContainerLib [label = "preload()"]

    SoftwareContainerLib -> ContainerAbstractInterface [label = "initialize()"]
    SoftwareContainerLib <-- ContainerAbstractInterface [label = "ReturnCode"]

    SoftwareContainerLib -> ContainerAbstractInterface [label = "create()"]
    SoftwareContainerLib <-- ContainerAbstractInterface [label = "ReturnCode"]

    SoftwareContainerLib -> ContainerAbstractInterface [label = "start()"]
    SoftwareContainerLib <-- ContainerAbstractInterface [label = "ReturnCode"]

    SoftwareContainerLib -> Gateway [label = "new()", note = "multiple calls"]
    SoftwareContainerLib <-- Gateway

    SoftwareContainerLib -> SoftwareContainerLib [label = "addGateway()", note = "multiple calls"]

    SoftwareContainerLib -> Gateway [label = "setContainer()", note = "multiple calls"]
    SoftwareContainerLib <-- Gateway

    Agent <-- SoftwareContainerLib [label = "ReturnCode"]

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

    SoftwareContainerLib -> Container [label = "initialize()"]
    Container -> Container [label = "createDirectory()"]
    Container -> Container [label = "createSharedMountPoint()"]
    SoftwareContainerLib <-- Container [label = "ReturnCode"]

    SoftwareContainerLib -> Container [label = "create()"]
    Container -> liblxc [label = "lxc_container_new()"]
    Container <-- liblxc [label = "container_struct"]
    === various operations on the lxc struct ===
    SoftwareContainerLib <-- Container [label = "ReturnCode"]

    SoftwareContainerLib -> Container [label = "start()"]
    === various operations on the lxc struct ===
    SoftwareContainerLib <-- Container [label = "ReturnCode"]


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


    Agent -> SoftwareContainerLib [label = "updateGatewayConfiguration()"]
    SoftwareContainerLib -> SoftwareContainerLib [label = "setGatewayConfigs()"]

    SoftwareContainerLib -> Gateway [label = "id()"]
    SoftwareContainerLib <-- Gateway [label = "ID"]

    SoftwareContainerLib -> Gateway [label = "setConfig()"]

    Gateway -> derived-gateway [label = "readConfigElement()"]
    Gateway <-- derived-gateway [label = "bool"]

    SoftwareContainerLib <-- Gateway [label = "bool"]

    SoftwareContainerLib -> Gateway [label = "isConfigured()"]
    SoftwareContainerLib <-- Gateway [label = "bool"]

    SoftwareContainerLib -> Gateway [label = "activate()"]
    SoftwareContainerLib <-- Gateway [label = "bool"]

    Agent <-- SoftwareContainerLib [label = "void", failed]
