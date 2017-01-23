/*
 * Copyright (C) 2016-2017 Pelagicore AB
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

#include "temperatureservicetodbusadapter.h"

TemperatureServiceToDBusAdapter::TemperatureServiceToDBusAdapter(TemperatureServiceImpl *ts,
                                                                 bool useSessionBus) :
    m_ts(ts)
{
    std::string busName = "com.pelagicore.TemperatureService";
    Gio::DBus::BusType busType = useSessionBus ? Gio::DBus::BUS_TYPE_SESSION
                                               : Gio::DBus::BUS_TYPE_SYSTEM;
    connect(busType, busName);
}

void TemperatureServiceToDBusAdapter::Echo(std::string argument,
                                           TemperatureServiceMessageHelper msg)
{
    std::cout << argument << std::endl;
    msg.ret(argument);
}

void TemperatureServiceToDBusAdapter::GetTemperature (TemperatureServiceMessageHelper msg)
{
    msg.ret(m_ts->getTemperature());
}

void TemperatureServiceToDBusAdapter::SetTemperature(double temperature,
                                                     TemperatureServiceMessageHelper msg)
{
    m_ts->setTemperature(temperature);
    TemperatureChanged_emitter(temperature);
    msg.ret();
}

