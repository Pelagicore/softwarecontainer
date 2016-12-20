The role of a launcher
======================

This section describes what typical integration actions are needed to integrate SoftwareContainer with
a launcher. For an overview of the general architecture involving a launcher and SoftwareContainer, see
:ref:`Design <design>`.

The assumed scenario in this section is that a launcher want to start an application inside the container.

The launcher should do the following:

 * Make the app home directory available inside the container.
 * Set the HOME environment variable in the container point to the above directory.

The above actions are performed by interacting with the SoftwareContainerAgent :ref:`D-Bus API <api>`.

Setting up a home directory and HOME
------------------------------------

By calling ``BindMount`` and passing a path on the host that will be mounted inside
the container at the location specified as the ``pathInContainer`` argument, a directory is
made available to any application started in the container later.

The path inside the container is intended to be set as the ``HOME`` environment variable inside the
container. The variable is set when calling ``Execute`` with an appropriate dictionary passed as
the  ``env`` argument.
