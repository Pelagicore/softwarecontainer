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
#include <dbus-c++/dbus.h>

#include "temperatureserviceinterface.h"
#include "periodictemperature.h"

int main()
{
    // Setup DBus
    DBus::BusDispatcher dispatcher;
    DBus::default_dispatcher = &dispatcher;
    DBus::Connection bus = DBus::Connection::SystemBus();

    // We use the $HOME to point to where we want to log messages
    std::string homeDir = getenv("HOME");
    if (homeDir.empty()) {
        std::cout << "No $HOME set, can't run" << std::endl;
        return 1;
    }

    std::cout << "App home dir is : " << homeDir << std::endl;
    TemperatureServiceInterface tsInterface(bus, homeDir + "/temperatureservice_client.log");

    // We use a different thread to set the temperature periodically
    // that way we don't block the main thread which is used by dbus
    // when we sleep.
    std::thread setterThread(setTemperaturePeriodically, &tsInterface);

    // Listen for DBus
    dispatcher.enter();

    return 0;
}
