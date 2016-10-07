
.. _integration-guidelines:

Integration guidelines
**********************

This chapter describes the workflow and major tasks and concepts involved in integrating SoftwareContainer
into a platform.

.. todo::

    This chapter work in progress and lacks a lot of content.

Introduction
============

The general steps involved in integration are:

* Writing service manifests for capabilities in the platform.
* Integrating a launcher.

Service manifests
=================

For details about format and content of service manifests, see :ref:`Service manifests <service-manifests>`
and :ref:`Gateways <gateways>`.

Service manifests should be installed in ``/etc/softwarecontainer/caps.d/<service-name>``.
