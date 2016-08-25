
/*
 * Copyright (C) 2016 Pelagicore AB
 *
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
 * WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR
 * BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES
 * OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
 * ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 *
 * For further information see LICENSE
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

    ReturnCode readConfigElement(const json_t *element) override;

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

