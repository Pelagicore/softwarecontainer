/*
 *   Copyright (C) 2014 Pelagicore AB
 *   All rights reserved.
 */
#include <stdio.h>
#include "pulsegateway.h"
#include "debug.h"
#include "jansson.h"
#include <libgen.h>

PulseGateway::PulseGateway(const std::string &gatewayDir, const std::string &containerName,
                           ControllerAbstractInterface *controllerInterface):
    Gateway(controllerInterface),
    m_api(0),
    m_context(0),
    m_mainloop(NULL),
    m_index(-1),
    m_enableAudio(false),
    m_controllerInterface(controllerInterface)
{
    m_socket = gatewayDir + "/pulse-" + containerName + ".sock";
}

PulseGateway::~PulseGateway()
{
}

bool PulseGateway::teardown()
{
    if (m_mainloop) {
        /* Unload module if module loaded successfully on startup */
        if (m_index != -1) {
            pa_threaded_mainloop_lock(m_mainloop);
            pa_context_unload_module(
                m_context,
                m_index,
                unloadCallback,
                this);
            pa_threaded_mainloop_wait(m_mainloop);
            pa_threaded_mainloop_unlock(m_mainloop);
        }

        /* Release pulse */
        pa_threaded_mainloop_lock(m_mainloop);
        pa_context_disconnect(m_context);
        pa_context_unref(m_context);
        pa_threaded_mainloop_unlock(m_mainloop);

        pa_threaded_mainloop_stop(m_mainloop);
        pa_threaded_mainloop_free(m_mainloop);
    }
    log_debug("pulse: Teardown complete");

    return true;
}

std::string PulseGateway::id()
{
    return "pulseaudio";
}

bool PulseGateway::setConfig(const std::string &config)
{
    bool success = true;
    ConfigError err = ConfigError::Ok;

    /* Check the value of the audio key of the config */
    std::string value = parseConfig(config.c_str(), "audio", &err);

    if (value == "true") {
        log_debug("Audio will be enabled\n");
        m_enableAudio = true;
    } else {
        log_debug("Audio will be disabled\n");
        if (err == ConfigError::BadConfig) {
            log_error("Malformed configuration file");
            success = false;
        }
        m_enableAudio = false;
    }

    if (m_enableAudio) {
        std::string var = "PULSE_SERVER";
        std::string val = "/gateways/" + socketName();
        success = m_controllerInterface->setEnvironmentVariable(var, val);
    }

    return success;
}

bool PulseGateway::activate()
{
    bool success = true;
    if (m_enableAudio) {
        success = connectToPulseServer();
    }

    return success;
}

bool PulseGateway::connectToPulseServer()
{
    bool success = true;

    /* Create mainloop */
    m_mainloop = pa_threaded_mainloop_new();
    pa_threaded_mainloop_start(m_mainloop);

    if (m_mainloop) {
        /* Set up connection to pulse server */
        pa_threaded_mainloop_lock(m_mainloop);
        m_api = pa_threaded_mainloop_get_api(m_mainloop);
        m_context = pa_context_new(m_api, "pulsetest");
        pa_context_set_state_callback(m_context, stateCallback, this);

        int err = pa_context_connect(
                  m_context,        /* context */
                  NULL,              /* default server */
                  PA_CONTEXT_NOFAIL, /* keep reconnection on failure */
                  NULL );            /* use default spawn api */

        if (err != 0) {
            success = false;
            log_error("pulse: Error code %d (%s)", err, pa_strerror(err));

            if (err == -1) {
                log_debug("Is the home directory set?");
            }
        }

        pa_threaded_mainloop_unlock(m_mainloop);
    } else {
        success = false;
        log_error("Failed to create pulse mainloop->");
    }

    return success;
}

std::string PulseGateway::socketName()
{
    char socket[1024];
    snprintf(socket, sizeof(socket), "%s", m_socket.c_str());
    return std::string(basename(socket));
}

void PulseGateway::loadCallback(pa_context *context, uint32_t index, void *userdata)
{
    PulseGateway *p = static_cast<PulseGateway *>(userdata);
    p->m_index = (int)index;
    int error = pa_context_errno(context);
    if (error != 0) {
        log_error("pulse: Error code %d (%s)", error, pa_strerror(error));
    }

    log_debug("pulse: Loaded module %d", p->m_index);

    pa_threaded_mainloop_signal(p->m_mainloop, 0);
}

void PulseGateway::unloadCallback(pa_context *context, int success, void *userdata)
{
    PulseGateway *p = static_cast<PulseGateway *>(userdata);

    if (success) {
        log_debug("pulse: Unloaded module %d", p->m_index);
    } else {
        log_debug("pulse: Failed to unload module %d", p->m_index);
    }

    pa_threaded_mainloop_signal(p->m_mainloop, 0);
}

void PulseGateway::stateCallback(pa_context *context, void *userdata)
{
    PulseGateway *p = static_cast<PulseGateway *>(userdata);
    char socket[1031]; /* 1024 + 7 for "socket=" */

    switch (pa_context_get_state(context)) {
    case PA_CONTEXT_READY:
        log_debug("Connection is up, loading module");
        snprintf(socket, sizeof(socket), "socket=%s", p->m_socket.c_str());
        pa_context_load_module(
            context,
            "module-native-protocol-unix",
            socket,
            loadCallback,
            userdata);
        break;
    case PA_CONTEXT_CONNECTING:
        log_debug("pulse: Connecting");
        break;
    case PA_CONTEXT_AUTHORIZING:
        log_debug("pulse: Authorizing");
        break;
    case PA_CONTEXT_SETTING_NAME:
        log_debug("pulse: Setting name");
        break;
    case PA_CONTEXT_UNCONNECTED:
        log_debug("pulse: Unconnected");
        break;
    case PA_CONTEXT_FAILED:
        log_debug("pulse: Failed");
        break;
    case PA_CONTEXT_TERMINATED:
        log_debug("pulse: Terminated");
        break;
    }
}

std::string PulseGateway::parseConfig(
    const std::string &config
    , const std::string &key
    , ConfigError *err)
{
    json_error_t  error;
    json_t       *root, *value;
    std::string ret = "";

    /* Get root JSON object */
    root = json_loads(config.c_str(), 0, &error);

    if (!root) {
        log_error("Error on line %d: %s\n", error.line, error.text);
        *err = ConfigError::BadConfig;
        goto cleanup_parse_json;
    }

    // Get string
    value = json_object_get(root, key.c_str());

    if (error.text == NULL) {
        *err = ConfigError::BadConfig;
        goto cleanup_parse_json;
    }

    if (!json_is_string(value)) {
        log_error("Value is not a string.");
        log_error("error: on line %d: %s\n", error.line, error.text);
        *err = ConfigError::BadConfig;
        json_decref(value);
        goto cleanup_parse_json;
    }

    ret = std::string(json_string_value(value));

    goto cleanup_parse_json;

cleanup_parse_json:
    if (root) {
        json_decref(root);
    }

    return ret;
}
