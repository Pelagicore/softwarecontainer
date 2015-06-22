/*
 *   Copyright (C) 2015 Pelagicore AB
 *   All rights reserved.
 */

#pragma once

#include "gateway.h"
#include "controllerinterface.h"
#include "systemcallinterface.h"


class CgroupsGateway: public Gateway
{
    LOG_DECLARE_CLASS_CONTEXT("CGRO", "Cgroups gateway");

public:
    CgroupsGateway(ControllerAbstractInterface &controllerInterface,
                    SystemcallAbstractInterface &systemcallInterface,
                    const std::string &containerName);

    ~CgroupsGateway() { }

    virtual std::string id();

    virtual bool setConfig(const std::string &config);

    virtual bool activate();

    virtual bool teardown();

private:
    bool parseSettingsFromConfig(const std::string &config);

    SystemcallAbstractInterface &m_systemcallInterface;
    std::string m_containerName;
    std::vector<std::string> m_settings;
    bool m_hasBeenConfigured;
};