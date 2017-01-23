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

#pragma once

#include "temperatureservice_dbus_stub.h"
#include "temperatureservice.h"

class TemperatureServiceImpl;

/*
 * Adapter between DBus and TemperatureService class
 */
class TemperatureServiceToDBusAdapter :
    public com::pelagicore::TemperatureService
{
public:
    TemperatureServiceToDBusAdapter(TemperatureServiceImpl *ts, bool useSessionBus);

    virtual void Echo (std::string argument, TemperatureServiceMessageHelper msg);
    virtual void GetTemperature (TemperatureServiceMessageHelper msg);
    virtual void SetTemperature (double temperature, TemperatureServiceMessageHelper msg);

private:
    TemperatureServiceImpl *m_ts;
};
