/*
 *   Copyright (C) 2015 Pelagicore AB
 *   All rights reserved.
 */

#pragma once

#include "gateway.h"
#include "controllerinterface.h"
#include "systemcallinterface.h"


/*! The cgroups gateway sets cgroups related settings for the container.
 *
 * The gateway configuration contains settings as key/value pairs and the setting
 * key and value will be applied using 'lxc-cgroup' as they are written in the
 * gateway config (the 'lxc.cgroup' prefix is added to all keys by the gateway).
 *
 * No syntax or other checks for correctness is performed on the key/value pairs,
 * see the lxc.container.conf man page for more details about legal settings.
 *
 * JSON format used in gateway config (as passed to setConfig():
 * \code{.js}
 * [
 *   {
 *     "setting": "memory.limit_in_bytes",
 *     "value": "128M"
 *   },
 *   {
 *     "setting": "cpu.shares",
 *     "value":  "256"
 *   }
 * ]
 * \endcode
 *
 * The root object is an array of seting key/value pair objects. Each key/value pair
 * must have the 'name' and 'value' defined. With the above example config the calls
 * to lxc-cgroup would set the following:
 * - lxc.cgroup.memory.limit_in_bytes to '128M'
 * - lxc.cgroup.cpu.shares to '256'
 *
 * It is an error to prepend the 'lxc.cgroup' prefix to settings in the config.
 */

class CgroupsGateway: public Gateway
{
    LOG_DECLARE_CLASS_CONTEXT("CGRO", "Cgroups gateway");

public:
    CgroupsGateway(ControllerAbstractInterface &controllerInterface,
                    SystemcallAbstractInterface &systemcallInterface,
                    const std::string &containerName);

    ~CgroupsGateway() { }

    /*!
     *  Implements Gateway::id
     */
    virtual std::string id();

    /*!
     *  Implements Gateway::setConfig
     *
     * Parses the gateway config, which will be applied by a call to activate()
     *
     * \param config JSON configuration object as string
     * \returns true if config could be parsed correctly, false otherwise
     */
    virtual bool setConfig(const std::string &config);

    /*!
     *  Implements Gateway::activate
     *
     * Calls lxc-cgroup to apply the settings from the gateway config one by one.
     *
     * \returns true if all calls to lxc-cgroup were successful, false otherwise
     */
    virtual bool activate();

private:
    bool parseSettingsFromConfig(const std::string &config);

    SystemcallAbstractInterface &m_systemcallInterface;
    std::string m_containerName;

    // Holds the settings parsed from the gateway config
    std::vector<std::string> m_settings;

    bool m_hasBeenConfigured;
};