Wayland setup
=============

In order to have applications access Wayland, one needs to enable the Wayland gateway, and possibly
give access to graphics hardware. Not all applications require direct access to the graphics
hardware, see :ref:`Wayland example <wayland-example>`. A reasonable capability for a Wayland
application would therefore include both the Wayland gateway and a configuration of the Device Node
gateway for any graphics hardware access needed.

Example
-------
Here is an example manifest defining Wayland access::

    {
        "version": "1",
        "capabilities": [{
            "name": "com.example.wayland-access",
            "gateways": [{
                "id": "wayland",
                "config": [{
                    "enabled": true
                }]
            }, {
                "id": "devicenode",
                "config": [{
                    "name": "/dev/dri/card0"
                }]
            }]
        }]
    }

