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

#include "temperatureservice.h"

TemperatureService::TemperatureService()
{
    
}

TemperatureService::~TemperatureService()
{

}

void TemperatureService::setAdapter(TemperatureServiceToDBusAdapter *adapter)
{
    m_adapter = adapter;
}

double TemperatureService::getTemperature()
{
    return m_temperature;
}

void TemperatureService::setTemperature(double temperature)
{
    m_temperature = temperature;
    
    if(m_adapter) {
        m_adapter->TemperatureChanged(temperature);
    }
}
