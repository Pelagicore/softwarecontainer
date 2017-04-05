File gateway
============

The File Gateway is used to expose individual files and directories from the host filesystem inside
the container.

ID
--

The ID used for the File gateway is: ``file``

Configuration
-------------

The paths inside the container has to be absolute. There is a check for not mounting over already
existing mount paths.

It is possible to supply the same host path in several configuration snippets to get the same file
mounted onto several locations in the container. However, It is not possible to supply the same
container path several times unless the host path is also the same. In those cases, the gateway will
merge the configurations and set the more permissive of the read-only settings for the file, with
read-write being more permissive than read-only.

Example configurations
----------------------

An example configuration can look like this::

    [
        {
            "path-host": "/tmp/someIPSocket",   // Path to the file in host's file-system
            "path-container": "/tmp/someIPSocket",   // Absolute path to the mount point in the container
            "read-only": false,  // if true, the file is accessible in read-only mode in the container
        }
    ]


