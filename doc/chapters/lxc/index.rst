SoftwareContainer using LXC
***************************

SoftwareContainer uses the Linux Containers (`LXC <https://linuxcontainers.org/>`_) project as its container backend. LXC uses namespaces and cgroups,
and has been supported in the Linux kernel since 2.6.24. 

LXC Template
------------
LXC expects a template that sets up a basic file system structure so that the container has something to boot into.
The LXC template ``SoftwareContainer`` uses does three things currently:

* Creates a basic rootfs with all directories one would expect
* Copies busybox into the rootfs, and populates ``/bin`` with all its aliases
* Adds some conditional options to the LXC configuration file

Create basic rootfs
^^^^^^^^^^^^^^^^^^^
The rootfs created is a basic FHS-like (`FHS <https://en.wikipedia.org/wiki/Filesystem_Hierarchy_Standard>`_)
structure, although stripped down, with the added ``/gateways`` directory. LXC template will also
create two paths, that will be substituted by CMake - ``${CMAKE_INSTALL_PREFIX}`` and
``${ADDITIONAL_FOLDER_MOUNTS}``. Furthermore, a root user and group will be created, and some
configuration options will be set in the following three areas:

* ``/etc/pulse/client.conf`` - tell pulse not to use shm
* ``/etc/machine-id`` - populated with a dbus-uuid
* ``/etc/resolv.conf`` - copied from host

``/lib64`` and ``/usr/lib64`` are also added to the rootfs - they will be empty unless they exist
in the host, in which case they will be bind mounted just like all other file systems, more on
that below in :ref:`LXC Configuration file <lxc_conf>`.

Copy and set up busybox
^^^^^^^^^^^^^^^^^^^^^^^
This step checks for busybox on the host, copies it into the rootfs for the container, then symlinks
all its functions to busybox in the ``/bin`` directory in the container - so that
``/bin/ls -> /?bin/busybox``.

There is an ongoing discussion on the need for busybox at all - this has implications on startup time as well as for the actual code.

Set up dynamic configuration options
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
This step adds some entries to the config file for LXC. It sets the location of the rootfs, and adds some mount entries,
namely the following:

* The directory containing ``init.lxc`` is bind-mounted to ``/usr/sbin``
* If ``$GATEWAY_DIR`` is set, bind mount its directory to ``/gateways`` and ``chmod 777`` it.

Full example
^^^^^^^^^^^^
This is the full template used.

.. literalinclude:: ../../../libsoftwarecontainer/lxc-softwarecontainer.in
    :language: bash

.. _lxc_conf:

LXC Configuration file
-----------------------

The configuration file contains three things: network setup, device and pty/tty allocation, and mount entries.

Network setup
^^^^^^^^^^^^^
The Network setup configuration is used when LXC creates a veth interface, connected to lxcbr0
(not set up here!), and for the network to be up.

Device and pty/tty allocation
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
LXC has a directive called "autodev", creates all needed devices automatically when set.
This is used in conjunction with telling LXC to allocate tty and pty devices.

Mount entries
^^^^^^^^^^^^^
The static mount entries tell LXC to bind mount ``/usr``, ``/lib``, ``/usr/lib`` and ``/proc``
into the container. These are then amended by the template when run.

Full example:
^^^^^^^^^^^^^
.. literalinclude:: ../../../libsoftwarecontainer/softwarecontainer.conf
    :language: bash

LXC API
-------
TBD

