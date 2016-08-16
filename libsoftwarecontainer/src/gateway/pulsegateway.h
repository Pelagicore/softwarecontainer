/*
 *   Copyright (C) 2014 Pelagicore AB
 *   All rights reserved.
 */
#pragma once

#include <pulse/pulseaudio.h>
#include "gateway.h"

/*! This SoftwareContainer gateway is responsible for setting up a connection to the
 *  PulseAudio server running on the host system. The gateway decides whether to
 *  connect to the PulseAudio server or not based on the configuration.
 *  If configured to enable audio, the gateway sets up a mainloop and then connects
 *  to the default PulseAudio server by calling pa_context_connect(). This is done
 *  during the activate() call.
 *  Once activate() has been called, the gateway listens to changes in the connection
 *  through the stateCallback function and, once the connection has been successfully
 *  set up, loads the module-native-protocol-unix PulseAudio module.
 *
 *  JSON format for configuration that enables audio:
 *  \code{.js}
 *  [
 *   {"audio": true}
 *  ]
 *  \endcode
 *
 *  A malformed configuration or a configuration that sets audio to false will simply
 *  disable audio and in such case, the gateway will not connect to the PulseAudio
 *  server at all.
 */
class PulseGateway :
    public Gateway
{
    LOG_DECLARE_CLASS_CONTEXT("PULS", "Pulse gateway");

    static constexpr const char *PULSE_AUDIO_SERVER_ENVIRONMENT_VARIABLE_NAME = "PULSE_SERVER";
    static constexpr const char *SOCKET_FILE_NAME = "pulse_socket";

public:
    static constexpr const char *ID = "pulseaudio";

    PulseGateway();
    ~PulseGateway();

    ReturnCode readConfigElement(const JSonElement &element) override;

    /*!
     *  Implements Gateway::activateGateway
     *
     *  If audio is to be enabled, then calling this function results in a call
     *  to connectToPulseServer.
     *
     * \returns true upon success (PulseAudio server connect call and mainloop
     *               setup successfully), false otherwise.
     */
    virtual bool activateGateway() override;

    /*!
     * Implements Gateway::teardownGateway
     */
    virtual bool teardownGateway() override;

private:
    bool m_enableAudio = false;
};

