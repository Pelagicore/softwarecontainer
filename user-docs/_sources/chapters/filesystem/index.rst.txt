
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
filesystem is automated based on a configuration file and a shell script
provided at startup. The process of creating the filesystem and bind mounting
different parts is spread out over several involved parties because of this.
Following is a rough description of the steps taken when setting up the
standard filesystem for SoftwareContainer:

1. The root filesystem directory is created and the
   directory is empty on creation.
2. It is setup by the ``lxc-softwarecontainer`` script, which is run when
   the container is created.
3. The ``lxc-softwarecontainer`` creates several basic directories
   creating a normal unix environment as defined by Filesystem Hierarchy
   Standard (FHS).
4. The filesystem is still mostly empty except for directories and some basic
   files such as ``/etc/passwd`` etc.
5. LXC then mounts some basic directories defined in the
   ``libsoftwarecontainer/softwarecontainer.conf``.
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

To enable the write buffer, set the ``writeBufferEnabled`` option in the
``com.pelagicore.SoftwareContainerAgent.Create(config)`` call.
This is done using the config parameter in specific, for example, this JSON
config::

    [{
        "writeBufferEnabled": true
    }]

The underlying mechanism of the write buffer is the ``OverlayFS`` which mounts a
``lower``, ``upper`` and ``work`` directory together into a mountpoint. Files
are written temporarily into the ``work`` directory while being worked on and
then into the ``upper`` directory. The ``upper`` directory is merged on top of
the ``lower`` directory. If a file from the ``lower`` filesystem is opened and
then edited, it will be written in the ``upper`` directory and the user of the
mountpoint will see only the changed file.

.. note:: Enabling the write buffer will enable write buffers for all
          filesystems, both the rootfs and the all bindmounted filesystems
          inside the container. Also, the filesystems will no longer be
          bindmounted technically, as bindmounting and overlayfs are mutually
          exclusive.

If a file is created in the ``lower`` filesystem, the file will be visible in
the merged filesystem, but it will not be part of the ``upper`` or ``work``
filesystems unless someone opens it from the ``merged`` filesystem, edits and
saves it.

Opening a file in ``upper`` or ``work`` does not create a file in the
``lower`` filesystem until it is synced to the lower filesystem. Syncing is
performed when a container is destroyed using the
``com.pelagicore.SoftwareContainerAgent.Destroy(id)`` call.

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

.. Note:: Non-directory types of files can not be mounted using overlayfs.
          These will automatically fall back on using the default behavior of 
          bind mounting the files into the filesystem of the container.

Temporary Filesystem
====================

The write buffers can be configured to use a separate temporary filesystem
(``tmpfs``) which can be limited in size. The ``tmpfs`` is mounted on top of
the containers temporary directory as soon as it's created and will remain
there until the container is destroyed. All ``overlayfs`` mounts and temporary
directories should be created inside this ``tmpfs`` mount.

The ``size`` of the ``tmpfs`` can also be limited using an extra configuration
option in the ``Create`` DBus call to the ``SoftwareContainerAgent``.
An example configuration would look like this::

    [{
        "writeBufferEnabled": true,
        "temporaryFileSystemWriteBufferEnabled": true,
        "temporaryFileSystemSize": 10485760
    }]

The ``temporaryFileSystemWriteBufferEnabled`` setting enables the ``tmpfs``
creation as described above, while the ``temporaryFileSystemSize`` variable
sets the size of the ``tmpfs`` in bytes.

.. Note:: The ``temporaryFileSystemSize`` parameter will not be parsed unless
          the ``temporaryFileSystemWriteBufferEnabled`` parameter is ``true``.
          The ``temporaryFileSystemSize`` is not required if the
          ``temporaryFileSystemWriteBufferEnabled`` is set to ``false`` or not
          added at all.

.. Note:: The ``tmpfs`` is shared between the upper and work directories in 
          ``overlayfs`` being mounted to the ``rootfs`` and all the 
          directories being bindmounted into a single instance of a container.
