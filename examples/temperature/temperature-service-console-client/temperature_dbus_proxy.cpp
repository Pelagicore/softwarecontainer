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

#include "temperature_dbus_proxy.h"

void com::pelagicore::TemperatureService::Echo(
        std::string arg_argument,
        const Gio::SlotAsyncReady &callback)
{
    Glib::VariantContainerBase base;
    Glib::Variant<Glib::ustring> params = Glib::Variant<Glib::ustring>::create(arg_argument);
    base = Glib::VariantContainerBase::create_tuple(params);

    m_proxy->call(
        "Echo",
        callback,
        base);
}

void com::pelagicore::TemperatureService::Echo_finish(
        std::string& out_unnamed_arg1,

        const Glib::RefPtr<Gio::AsyncResult>& result)
{
    Glib::VariantContainerBase wrapped;
    wrapped = m_proxy->call_finish(result);

    Glib::Variant<std::string> unnamed_arg1_variant;
    wrapped.get_child(unnamed_arg1_variant,0);
    out_unnamed_arg1 = unnamed_arg1_variant.get();

}

void com::pelagicore::TemperatureService::GetTemperature(
        const Gio::SlotAsyncReady &callback)
{
    Glib::VariantContainerBase base;

    m_proxy->call(
        "GetTemperature",
        callback,
        base);
}

void com::pelagicore::TemperatureService::GetTemperature_finish(
        double& out_unnamed_arg0,

        const Glib::RefPtr<Gio::AsyncResult>& result)
{
    Glib::VariantContainerBase wrapped;
    wrapped = m_proxy->call_finish(result);

    Glib::Variant<double> unnamed_arg0_variant;
    wrapped.get_child(unnamed_arg0_variant,0);
    out_unnamed_arg0 = unnamed_arg0_variant.get();

}

void com::pelagicore::TemperatureService::SetTemperature(
        double arg_temperature,
        const Gio::SlotAsyncReady &callback)
{
    Glib::VariantContainerBase base;
    Glib::Variant<double> params = Glib::Variant<double>::create(arg_temperature);
    base = Glib::VariantContainerBase::create_tuple(params);

    m_proxy->call(
        "SetTemperature",
        callback,
        base);
}

void com::pelagicore::TemperatureService::SetTemperature_finish(const Glib::RefPtr<Gio::AsyncResult>& result)
{
    Glib::VariantContainerBase wrapped;
    wrapped = m_proxy->call_finish(result);
}

void com::pelagicore::TemperatureService::handle_signal(const Glib::ustring& /*sender_name*/,
                                                        const Glib::ustring& signal_name,
                                                        const Glib::VariantContainerBase& parameters)
{
    if (signal_name == "TemperatureChanged") {
            if (parameters.get_n_children() != 1) { return; }
            Glib::Variant<double > base_unnamed_arg0;
            parameters.get_child(base_unnamed_arg0, 0);
            double p_unnamed_arg0;
            p_unnamed_arg0 = base_unnamed_arg0.get();
            TemperatureChanged_signal.emit((p_unnamed_arg0));
    }
}

void com::pelagicore::TemperatureService::createForBus(Gio::DBus::BusType busType,
                                                       Gio::DBus::ProxyFlags proxyFlags,
                                                       const std::string &name,
                                                       const std::string &objectPath,
                                                       const Gio::SlotAsyncReady &slot) {
    Gio::DBus::Proxy::create_for_bus(busType,
                                     name,
                                     objectPath,
                                     "com.pelagicore.TemperatureService",
                                     slot,
                                     Glib::RefPtr<Gio::DBus::InterfaceInfo>(),
                                     proxyFlags);
}

Glib::RefPtr<com::pelagicore::TemperatureService>
com::pelagicore::TemperatureService::createForBusFinish (Glib::RefPtr<Gio::AsyncResult> result) {
    Glib::RefPtr<Gio::DBus::Proxy> proxy = Gio::DBus::Proxy::create_for_bus_finish (result);
    com::pelagicore::TemperatureService *p = new com::pelagicore::TemperatureService (proxy);
    return Glib::RefPtr<com::pelagicore::TemperatureService> (p);
}
