/*
 *   Copyright (C) 2014 Pelagicore AB
 *   All rights reserved.
 */
#ifndef PAMINTERFACE_H
#define PAMINTERFACE_H

#include "pamproxy.h"
#include "pamabstractinterface.h"

/*! PAMInterface is a D-Bus implementation of PAMAbstractInterface
 */
class PAMInterface :
    public com::pelagicore::PAM_proxy,
    public DBus::IntrospectableProxy,
    public DBus::ObjectProxy,
    public PAMAbstractInterface {
public:
    PAMInterface(DBus::Connection &connection);

    /*! Implements PAMAbstractInterface::registerClient
     */
    virtual void registerClient(const std::string &cookie, const std::string &appId);

    /*! Implements PAMAbstractInterface::unregisterClient
     */
    virtual void unregisterClient(const std::string &cookie);

    /*! Implements PAMAbstractInterface::updateFinished
     */
    virtual void updateFinished(const std::string &cookie);
};

#endif /* PAMINTERFACE_H */
