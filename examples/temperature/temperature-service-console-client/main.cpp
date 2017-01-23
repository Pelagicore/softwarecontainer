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

#include <thread>
#include <unistd.h>

#include "temperatureserviceinterface.h"


class PeriodicTemperature
{
public:
    PeriodicTemperature() : m_temperature(0.0) {}

    bool setTemperaturePeriodically() {
           if(m_temperature > 40) {
               m_temperature = 0;
           }

           m_temperature = m_temperature + 0.5;
           m_interface.setTemperature(m_temperature);
           m_interface.getTemperature();
           return true;
    }

private:
    TemperatureServiceInterface m_interface;
    double m_temperature;
};

int main()
{
    Glib::init();
    Gio::init();

    auto loop = Glib::MainLoop::create();

    PeriodicTemperature tsInterface;
    auto callbackFunction = sigc::mem_fun(&tsInterface,
                                          &PeriodicTemperature::setTemperaturePeriodically);

    Glib::signal_timeout().connect(callbackFunction, 1000);
    loop->run();

    return 0;
}
