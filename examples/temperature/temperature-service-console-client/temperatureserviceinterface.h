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

#include "TemperatureServiceConsoleClient_dbuscpp_proxy.h"
#include "temperatureinterface.h"

/*! Adapter between DBus and TemperatureService_proxy class
 */

class TemperatureServiceInterface:
    public com::pelagicore::TemperatureService_proxy,
    public DBus::IntrospectableProxy,
    public DBus::ObjectProxy,
    public TemperatureInterface
{
public:
    TemperatureServiceInterface(DBus::Connection &connection, std::string logfilepath);
    std::string echo(const std::string &argument);
    double getTemperature();
    bool setTemperature(const double &temperature);
    void TemperatureChanged(const double& argin0);
private:
    std::string m_logfilepath;
};

