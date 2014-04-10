/*
 *   Copyright (C) 2014 Pelagicore AB
 *   All rights reserved.
 */
#ifndef PULSEGATEWAY_H
#define PULSEGATEWAY_H

#include <pulse/pulseaudio.h>
#include "gateway.h"
#include "systemcallinterface.h"

/*! Pulse audio functionality for Pelagicontain
 */
class PulseGateway:
    public Gateway
{
public:
    PulseGateway(
        const std::string &gatewayDir
        , const std::string &containerName
        , ControllerAbstractInterface *controllerInterface);
    ~PulseGateway();

    /*!
     *  Implements Gateway::id
     */
    virtual std::string id();

    /*!
     *  Implements Gateway::setConfig
     */
    virtual bool setConfig(const std::string &config);

    /*!
     *  Implements Gateway::activate
     */
    virtual bool activate();

    /*! Implements Gateway::environment
     */
    virtual std::string environment();

private:

    enum ConfigError {Ok, BadConfig};

    /*! Creates a mainloop, sets a PulseAudio context, and sets callbacks for PulseAudio.
     *
     * \return true Upon success
     * \return false Upon failure
     */
    bool connectToPulseServer();

    /*! Static callback called by PulseAudio when module has been loaded
     *
     * \param context The pa_context passed from PulseAudio
     * \param index The index of the loaded module
     * \param userdata Pointer to the calling PulseGateway object
     *
     * \return true Upon success
     * \return false Upon failure
     */
    static void loadCallback(pa_context *context, uint32_t idx, void *userdata);

    /*! Static callback called by PulseAudio when module has been unloaded
     *
     * \param context The pa_context passed from PulseAudio
     * \param index The index of the loaded module
     * \param userdata Pointer to the calling PulseGateway object
     *
     * \return true Upon success
     * \return false Upon failure
     */
    static void unloadCallback(pa_context *context, int success, void *userdata);

    /*! Static callback called by PulseAudio when the PulseAudio server has changed state
     *
     * \param context The pa_context passed from PulseAudio
     * \param index The index of the loaded module
     * \param userdata Pointer to the calling PulseGateway object
     *
     * \return true Upon success
     * \return false Upon failure
     */
    static void stateCallback(pa_context *c, void *userdata);

    /*! Parse the JSON configuration passed down from Platform Access Manager
     *
     * Parses the configuration and looks up the value for the key passed as argument.
     *
     * \param config The JSON string containing the configuration
     * \param key The key to look up.
     * \return std::string  Value belonging to key
     * \return Empty string  Upon failure
     */
    std::string parseConfig(
        const std::string &config
        , const std::string &key
        , ConfigError *err = 0);

    /*! Returns the name of the socket, i.e. the filename part of the path
     *  to the socket.
     *
     * \return std::string The socket name
     */
    std::string socketName();

    pa_mainloop_api *m_api;
    pa_context *m_context;
    pa_threaded_mainloop *m_mainloop;
    std::string m_socket;
    int m_index;
    bool m_enableAudio;
    ControllerAbstractInterface *m_controllerInterface;
};

#endif /* PULSEGATEWAY_H */
