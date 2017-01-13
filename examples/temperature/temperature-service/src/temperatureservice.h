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

#include <sys/types.h>
#include "temperatureservicetodbusadapter.h"

class TemperatureServiceToDBusAdapter;

/*
 * The TemperatureService class is where you implement
 * your service, this is the work horse.
 */

class TemperatureService {
public:
    /*
     * @brief Sets temperature to 0.0 per default.
     */
    TemperatureService();
    ~TemperatureService();

    void setAdapter(TemperatureServiceToDBusAdapter *adapter);

    /*
     * @brief Return the current temperature
     * @return The current set temperature
     */
    double getTemperature();

    /*
     * @brief Set the temperature
     * @param temperature the desired temperature to set
     */
    void setTemperature(double temperature);

private:
    double m_temperature = 0;
    TemperatureServiceToDBusAdapter *m_adapter = nullptr;
};
