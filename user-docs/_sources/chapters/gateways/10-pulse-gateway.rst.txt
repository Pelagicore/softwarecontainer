PulseAudio gateway
==================

The PulseAudio Gateway is used to provide access to the host system PulseAudio server.

The gateway looks for the ``PULSE_SERVER`` environment variable, which is assumed
to be a socket, mounts the socket into the container, and sets the ``PULSE_SERVER`` variable inside
the container to the location of the mounted socket.

If the pulseaudio server is not on a socket but on a network, then one should not use this gateway,
but rather a combination of the network gateway (for access to the network resource) and the
environment gateway (for setting ``PULSE_SERVER`` to the appropriate URI).

ID
--

The ID used for the PulseAudio gateway is: ``pulseaudio``

Configuration
-------------

Example configuration enabling audio::

    [
        { "audio": true }
    ]

A malformed configuration or a configuration that sets audio to false will simply
disable audio and in such case, the gateway will not connect to the PulseAudio
server at all.

The configuration can be set several times, but once it has been set to ``true``
it will retain its value.


