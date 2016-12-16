PulseAudio gateway
==================

The PulseAudio Gateway is used to provide access to the host system PulseAudio server.

This gateway is responsible for setting up a connection to the
PulseAudio server running on the host system. The gateway decides whether to
connect to the PulseAudio server or not based on the configuration.

When configured to enable audio, the gateway sets up a mainloop and then connects
to the default PulseAudio server by calling ``pa_context_connect()``. This is done
during the ``activate()`` phase.

Once ``activate`` has been initiated, the gateway listens to changes in the connection
through the ``stateCallback`` function and, once the connection has been successfully
set up, loads the ``module-native-protocol-unix`` PulseAudio module.

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


