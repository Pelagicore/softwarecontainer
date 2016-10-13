
.. _filesystems:

Filesystems
***********

The filesystem behavior and layout is documented in this chapter. The document
discusses how the container filesystem is created, where the different mount 
points are pulled in from and the mechanisms that changes the behavior of the 
filesystem. 


Standard filesystem
===================

Each container has a rootfs that is bind mounted as the containers root
filesystem of said container. The creation and bind mounting of the root
filesystem is automated by LXC based on a configuration file and a shell script
provided at startup. The process of creating the filesystem and bind mounting
different parts is spread out over several involved parties because of this.

1. The root filesystem directory is created and the
   directory is empty on creation.
2. It is setup by the ``lxc-softwarecontainer.in`` script, which is run by
   lxc init when the container is created.
3. The ``lxc-softwarecontainer.in``  creates several basic directories
   creating a normal unix environment as defined by Filesystem Hierarchy
   Standard (FHS).
4. The filesystem is still mostly empty except for directories and some basic
   files such as ``/etc/passwd`` etc.
5. LXC then mounts some basic directories defined in the
   ``libsoftwarecontainer/softwarecontainer.conf`` (copied to
   ``/usr/share/lxc/conf`` on installation).
6. On top of this, non standard mountpoints are also used to mount for example
   the gateway directories etc. inside the filesystem. These are defined using
   :ref:`BindMountFolderInContainer <dbus-api>`, or mounted by
   the different gateways.

The basic filesystem looks like this on startup::

	.
	├── config
	└── rootfs
	    ├── bin
	    ├── dev
	    ├── etc
	    │   ├── group
	    │   ├── machine-id
	    │   ├── passwd
	    │   ├── pulse
	    │   │   └── client.conf
	    │   └── resolv.conf
	    ├── gateways
	    ├── home
	    ├── lib
	    ├── lib64
	    ├── proc
	    ├── root
	    ├── sbin
	    ├── tmp
	    └── usr
	       	├── lib
	       	├── lib64
	       	├── local
	       	└── sbin


Mountpoints
===========

Mount points can be created anywhere inside the container filestructure using
the :ref:`BindMountFolderInContainer <dbus-api>`  and gateway configurations
which can mount separate files inside the container filesystem.


Write Buffered Filesystems
==========================

A write buffer can be enabled on containers where it is known that the
application may be misbehaving towards the underlying filesystem. This can be
overly high number of writes leading to an early death of storage devices
for example. Enabling the write buffer will provide a layer of
protection for the filesystem by only allowing a final write of the changes in
the upper layer when the container is shutting down.

To enable the write buffer, set the ``enableWriteBuffer`` option in the
``com.pelagicore.SoftwareContainerAgent.CreateContainer(prefix, config)`` call.
This is done using the config parameter in specific, for example, this JSON
config::

    [{
        "enableWriteBuffer": true
    }]

The underlying mechanism of the write buffer is the ``OverlayFS`` which mounts a
``lower``, ``upper`` and ``work`` directory together into a mountpoint. Files
are written temporarily into the ``work`` directory while being worked on and
then into the ``upper`` directory. The ``upper`` directory is merged on top of
the ``lower`` directory. If a file from the ``lower`` filesystem is opened and
then edited, it will be written in the ``upper`` directory and the user of the
mountpoint will see only the changed file.

If a file is created in the ``lower`` filesystem, the file will be visible in
the merged filesystem, but it will not be part of the ``upper`` or ``work``
filesystems unless someone opens it from the ``merged`` filesystem, edits and
saves it.

Opening a file in ``upper`` or ``work`` does not create a file in the 
``lower`` filesystem until it is synced to the lower filesystem.

The ``upper`` directory is a temporarily created directory in the ``/tmp``
filesystem. The ``work`` is also a temporary filesystem.

.. blockdiag::
    :alt: "Description of OverlayFS layering"

    diagram { 
        orientation = portrait

        Work -> Upper -> Lower;
    }

When the container is shutdown and the mountpoints are cleaned up, the
upper filesystem is copied into the lower filesystem causing the filesystem
changes performed during its runtime to be merged into the lower layers. 

