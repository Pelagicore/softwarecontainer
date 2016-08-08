/*
 *   Copyright (C) 2014 Pelagicore AB
 *   All rights reserved.
 */
#ifndef GENERATORS_H
#define GENERATORS_H

#include <string>
#include "softwarecontainer-common.h"

/*! \brief  Generator functions
 *  \file   generators.h
 *
 *  Various helper functions for generating things such as network interface
 *  names, IP addresses and container names. By using these functions, unique
 *  names are ensured
 */
class Generator
{

    LOG_DECLARE_CLASS_CONTEXT("GEN", "Generator");

public:
    /*! \brief Generate an IP address
     *
     * Calling this function will generate a new IP address. A counter is kept in
     * /tmp to minimize the risk of collissons
     *
     * \param ip_addr_net A 24 bit network portion of an IP address
     * \return A string representing an IP address
     */
    std::string gen_ip_addr(const char *ip_addr_net);

    /*! \brief Generate a container name
     *
     * Generate a container name based on the current time in milliseconds
     *
     * \return name upon success
     * \return NULL upon failure
     */
    static std::string gen_ct_name();

private:
    int counter = 0;

};

#endif /* GENERATORS_H */
