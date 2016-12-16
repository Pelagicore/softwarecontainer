
Wayland gateway
===============

The Wayland Gateway is used to provide access to the hsot system Wayland server.

Please note that the Wayland Gateway only provides access to the Wayland
socket and sets the ``XDG_RUNTIME_DIR`` accordingly. For a Wayland
*capability*, it is also necessary to ensure that the contained application
has access to the graphics hardware devices and associated libraries. Access
to graphics hardware is **not** setup by the Wayland gateway.

ID
--

The ID used for the Wayland gateway is: ``wayland``

Configuration
-------------

Example configuration enabling Wayland::

    [
        { "enabled": true }
    ]

The configuration of the Wayland gateway can be set several times, but once it has been set to ``true``
it will retain its value.
