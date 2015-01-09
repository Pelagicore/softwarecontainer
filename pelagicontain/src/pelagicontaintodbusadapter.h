/*
 *   Copyright (C) 2014 Pelagicore AB
 *   All rights reserved.
 */
#ifndef PELAGICONTAINTODBUSADAPTER_H
#define PELAGICONTAINTODBUSADAPTER_H

#include <dbus-c++/dbus.h>

#include "dbusadaptor.h"
#include "pelagicontain.h"

class PelagicontainToDBusAdapter :
    public com::pelagicore::Pelagicontain_adaptor
{
public:
    PelagicontainToDBusAdapter(Pelagicontain &pc);
    virtual ~PelagicontainToDBusAdapter()
    {
    }
    virtual std::string Echo(const std::string &argument);
    virtual void Launch(const std::string &appId);
    virtual void LaunchCommand(const std::string &commandLine);
    virtual void Update(const std::map<std::string, std::string> &configs);
    virtual void SetContainerEnvironmentVariable(const std::string &var, const std::string &val);
    virtual void Shutdown();

private:
    Pelagicontain *m_pelagicontain;
};

#endif /* PELAGICONTAINTODBUSADAPTER_H */
