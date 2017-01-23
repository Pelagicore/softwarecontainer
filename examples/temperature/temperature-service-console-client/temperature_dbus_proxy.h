/*
 * Copyright (C) 2017 Pelagicore AB
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
#include <string>
#include <glibmm.h>
#include <giomm.h>

namespace com {
namespace pelagicore {

class TemperatureService : public Glib::ObjectBase {
public:
    static void createForBus (Gio::DBus::BusType busType,
                              Gio::DBus::ProxyFlags proxyFlags,
                              const std::string &name,
                              const std::string &objectPath,
                              const Gio::SlotAsyncReady &slot);

    static Glib::RefPtr<TemperatureService> createForBusFinish (Glib::RefPtr<Gio::AsyncResult> result);

    void Echo (
        std::string argument,
        const Gio::SlotAsyncReady &slot);

    void Echo_finish (
        std::string& unnamed_arg1,
        const Glib::RefPtr<Gio::AsyncResult>& res);

    void GetTemperature (
        const Gio::SlotAsyncReady &slot);

    void GetTemperature_finish (
        double& unnamed_arg0,
        const Glib::RefPtr<Gio::AsyncResult>& res);

    void SetTemperature (
        double temperature,
        const Gio::SlotAsyncReady &slot);

    void SetTemperature_finish (
        const Glib::RefPtr<Gio::AsyncResult>& res);

    sigc::signal<void, double > TemperatureChanged_signal;

    void reference() {}
    void unreference() {}
    void handle_signal (const Glib::ustring& sender_name, const Glib::ustring& signal_name, const Glib::VariantContainerBase& parameters);

private:
    TemperatureService (Glib::RefPtr<Gio::DBus::Proxy> proxy) : Glib::ObjectBase() {
        this->m_proxy = proxy;
        this->m_proxy->signal_signal().connect(sigc::mem_fun(this, &TemperatureService::handle_signal));
    }
    Glib::RefPtr<Gio::DBus::Proxy> m_proxy;
};
}// pelagicore
}// com
