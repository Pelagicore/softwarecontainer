
.. _service-manifests:

Service manifests
*****************

This chapter describes the format, structure, and some of the content in
the service manifests.

For information about how to work with the manifest files, e.g. where to
install them, and how they are used in the overall context etc.,
see :ref:`Integration guidelines <integration-guidelines>`.

Introduction
============

Service manifests are JSON files mapping capabilities to gateway
configurations. In other words, service manifests describe what
exact gateway configurations are needed to configure the relevant
gateways for a specific service in order to realize a capability.
The service manifests typically contain the definitions of all the capabilities
relevant for a service, although it is possible to split the definitions
over multiple files if needed.

For example, a platform might have a capability `com.acme.SomeResource`
defined. This capability requires access to service `A`. Service `A`
will then need to deploy a service manifest which contains all gateway
configurations needed for the `com.acme.SomeResource` capability.

A service manifest might also contain gateway configurations for more
than one capability.

More than one service might be needed to realize a capability, meaning that
each relevant service would need to be integrated in the above mentioned way.
For example, if the capability `com.acme.SomeResource` also requires access
to service `B` as well as `A`, both the services needs to deploy service
manifests and provide their respective gateway configurations needed to
realize the capability.

Manifest format
===============

A service manifest is a text file containing JSON. It is required to define
a version, and an array "capabilities" which contains a
"name" key with the capability name as value, and a "gateways" key which
contains an array of objects each defining a gateway configuration identified
with a gateway ID.

Example
-------

This section describes a full service manifest, and then goes through each part
separately in more detail.

Full manifest
^^^^^^^^^^^^^

Below is an example of a service manifest::

 {
    "version": "1",
    "capabilities": [
        {
            "name": "com.acme.SomeResource",
            "gateways": [
                {
                    "id": "example-gateway-id-1",
                    "config": [
                        {
                            "config-part1": []
                        },
                        {
                            "config-part2": [
                                {
                                    "config-element1": "on",
                                    "config-element2": "off",
                                    "config-element3": "optionA",
                                    "config-element4": "optionB"
                                }
                            ]
                        }
                    ]
                },
                {
                    "id": "example-gateway-id-2",
                    "config": []
                }
            ]
        },
        {
            "name": "com.acme.AnotherResource",
            "gateways": [
                {
                    "id": "example-gateway-id-1",
                    "config": []
                }
            ]
        }
    ]
 }


Breakdown into parts
^^^^^^^^^^^^^^^^^^^^

In the above manifest, there is a list of `capabilities` objects defined by the ``capabilities`` key::

 "capabilities": [
     ...
 ]

Each `capability` object has the keys ``name`` and ``gateways``::

 {
     "name": "com.acme.SomeResource",
     "gateways": [
         ...
     ]
 }

Where the ``name`` key has the capability name as value, and the ``gateways`` key has a list of objects
associating gateway IDs with gateway configs::

 {
     "id": "example-gateway-id-1",
     "config": [
         ...
     ]
 }

Where the ``config`` key contains gateway configuration. The exact value of the ``config`` key,
is described for each gateway, see :ref:`Gateways <gateways>`.
