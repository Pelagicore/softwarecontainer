Device node gateway
===================

The Device Node Gateway is used to provide access to host system device nodes.

ID
--

The ID used for the Device node gateway is: ``devicenode``

Configuration
-------------

The configuration consists of a root list consisting of individual devices. Each device contains the
following fields:

- ``name`` The name of the device, with or without path. This is passed verbatim to ``mknod``
- ``mode`` Permission mode, passed verbatim to ``chmod``

In the case where the same device node is configured more than once, the most permissive mode
is chosen per user type.

Example configurations
----------------------

An example configuration can look like this::

    [
        {
            "name":  "/dev/dri/card0"
        },
        {
            "name":  "/dev/galcore",
            "mode":  "722"
        },
        {
            "name":  "/dev/galcore",
            "mode":  "600"
        }
    ]

In the last example shown above the mode for ``/dev/galcore`` will be set to 600.


