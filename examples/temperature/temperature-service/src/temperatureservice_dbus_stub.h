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
#include "temperatureservicemessagehelper.h"

namespace com {
namespace pelagicore {
class TemperatureService {
public:
TemperatureService ();
void connect (Gio::DBus::BusType, std::string);
protected:
virtual void Echo (
    std::string argument,
    const TemperatureServiceMessageHelper msg) = 0;
virtual void GetTemperature (
    const TemperatureServiceMessageHelper msg) = 0;
virtual void SetTemperature (
    double temperature,
    const TemperatureServiceMessageHelper msg) = 0;

void TemperatureChanged_emitter(double);
sigc::signal<void, double > TemperatureChanged_signal;

void on_bus_acquired(const Glib::RefPtr<Gio::DBus::Connection>& connection,
                     const Glib::ustring& /* name */);

void on_name_acquired(const Glib::RefPtr<Gio::DBus::Connection>& /* connection */,
                      const Glib::ustring& /* name */);

void on_name_lost(const Glib::RefPtr<Gio::DBus::Connection>& connection,
                  const Glib::ustring& /* name */);

void on_method_call(const Glib::RefPtr<Gio::DBus::Connection>& /* connection */,
                   const Glib::ustring& /* sender */,
                   const Glib::ustring& /* object_path */,
                   const Glib::ustring& /* interface_name */,
                   const Glib::ustring& method_name,
                   const Glib::VariantContainerBase& parameters,
                   const Glib::RefPtr<Gio::DBus::MethodInvocation>& invocation);

void on_interface_get_property(Glib::VariantBase& property,
                                       const Glib::RefPtr<Gio::DBus::Connection>& connection,
                                       const Glib::ustring& sender,
                                       const Glib::ustring& object_path,
                                       const Glib::ustring& interface_name,
                                       const Glib::ustring& property_name);

bool on_interface_set_property(
       const Glib::RefPtr<Gio::DBus::Connection>& connection,
       const Glib::ustring& sender,
       const Glib::ustring& object_path,
       const Glib::ustring& interface_name,
       const Glib::ustring& property_name,
       const Glib::VariantBase& value);

private:
bool emitSignal(const std::string& propName, Glib::VariantBase& value);

guint connectionId, registeredId;
Glib::RefPtr<Gio::DBus::NodeInfo> introspection_data;
Glib::RefPtr<Gio::DBus::Connection> m_connection;
std::string m_objectPath;
std::string m_interfaceName;
};
}// pelagicore
}// com

