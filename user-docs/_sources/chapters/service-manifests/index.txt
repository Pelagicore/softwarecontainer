
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
The service manifests contain the definitions of all the capabilities
relevant for a service.

For example, a platform might have a capability `com.acme.SomeResource`
defined. This capability requires access to service `A`. Service `A`
will then need to deploy a service manifest which contains all gateway
configurations needed for the `com.acme.SomeResource` capability.

A service manifest might also contain gateway configurations for more
capabilities.

More than one service might be needed to fulfill a capability, meaning that
each relevant service would need to be integrated in the above mentioned way.

Manifest format
===============

A service manifest is a text file containing JSON. It is required to define
a version set to "1", and an array "capabilities" which contains a
"name" key with the capability name as value, and a "gateways" key which
contains an array of objects each defining a gateway configuration identified
with a gateway ID.

Examples
--------

Below is an example of a simple service manifest::

    {
      "version": "1",
      "capabilities": {
        "com.acme.SomeResource": {
          "gateways": [
            {
              "id": "dummy-gw1",
              "config": {}
            },
            {
              "id": "dummy-gw2",
              "config": {}
            }
          ]
        },
        "com.acme.AnotherResource": {
          "gateways": []
        }
      }
    }

In this manifest, there is one capability named `com.acme.SomeResource`
which defines gateway configurations for the two gateways `dummy-gw1` and
`dummy-gw2`. In this case the configurations for both gateways are empty
(but valid) JSON. Exact content will depend on what gateways are involved.
There is also a second capability named `com.acme.AnotherResource` with
a gateway object with an empty array. The manifest also contains the required
version element.
