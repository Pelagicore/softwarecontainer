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

#include "temperatureservice_dbus_stub.h"

Glib::ustring interfaceXml0 = R"XML_DELIMITER(
<?xml version="1.0" encoding="UTF-8" ?>
<node name="/com/pelagicore/TemperatureService">
    <interface name="com.pelagicore.TemperatureService">
        <method name="Echo">
            <arg direction="in" type="s" name="argument"/>
            <arg direction="out" type="s"/>
        </method>
        <method name="GetTemperature">
            <arg direction="out" type="d"/>
        </method>
        <method name="SetTemperature">
            <arg direction="in" type="d" name="temperature"/>
            <annotation name="org.freedesktop.DBus.Method.NoReply" value="true"/>
        </method>
        <signal name="TemperatureChanged">
            <arg direction="out" type="d"/>
        </signal>
    </interface>
</node>
)XML_DELIMITER";

com::pelagicore::TemperatureService::TemperatureService () :
    connectionId(0),
    registeredId(0),
    m_objectPath("/com/pelagicore/TemperatureService"),
    m_interfaceName("com.pelagicore.TemperatureService") {
    TemperatureChanged_signal.connect(sigc::mem_fun(this, &TemperatureService::TemperatureChanged_emitter));
}

void com::pelagicore::TemperatureService::connect (
    Gio::DBus::BusType busType,
    std::string name)
{
    try {
            introspection_data = Gio::DBus::NodeInfo::create_for_xml(interfaceXml0);
    } catch(const Glib::Error& ex) {
            g_warning("Unable to create introspection data: ");
            g_warning(std::string(ex.what()).c_str());
            g_warning("\n");
    }
    connectionId = Gio::DBus::own_name(busType,
                                       name,
                                       sigc::mem_fun(this, &TemperatureService::on_bus_acquired),
                                       sigc::mem_fun(this, &TemperatureService::on_name_acquired),
                                       sigc::mem_fun(this, &TemperatureService::on_name_lost));
}

void com::pelagicore::TemperatureService::on_method_call(const Glib::RefPtr<Gio::DBus::Connection>& /* connection */,
                   const Glib::ustring& /* sender */,
                   const Glib::ustring& /* object_path */,
                   const Glib::ustring& /* interface_name */,
                   const Glib::ustring& method_name,
                   const Glib::VariantContainerBase& parameters,
                   const Glib::RefPtr<Gio::DBus::MethodInvocation>& invocation)
{
    if (method_name.compare("Echo") == 0) {
        Glib::Variant<Glib::ustring > base_argument;
        parameters.get_child(base_argument, 0);
        Glib::ustring p_argument;
        p_argument = base_argument.get();

        Echo(
            Glib::ustring(p_argument),
            TemperatureServiceMessageHelper(invocation));
    }
    if (method_name.compare("GetTemperature") == 0) {
        GetTemperature(
            TemperatureServiceMessageHelper(invocation));
    }
    if (method_name.compare("SetTemperature") == 0) {
        Glib::Variant<double > base_temperature;
        parameters.get_child(base_temperature, 0);
        double p_temperature;
        p_temperature = base_temperature.get();

        SetTemperature(
            (p_temperature),
            TemperatureServiceMessageHelper(invocation));
    }
}

void com::pelagicore::TemperatureService::on_interface_get_property(Glib::VariantBase& /*property*/,
                                                                    const Glib::RefPtr<Gio::DBus::Connection>& /*connection*/,
                                                                    const Glib::ustring& /*sender*/,
                                                                    const Glib::ustring& /*object_path*/,
                                                                    const Glib::ustring& /*interface_name*/,
                                                                    const Glib::ustring& /*property_name*/) {

}

bool com::pelagicore::TemperatureService::on_interface_set_property(const Glib::RefPtr<Gio::DBus::Connection>& /*connection*/,
                                                                    const Glib::ustring& /*sender*/,
                                                                    const Glib::ustring& /*object_path*/,
                                                                    const Glib::ustring& /*interface_name*/,
                                                                    const Glib::ustring& /*property_name*/,
                                                                    const Glib::VariantBase& /*value*/) {
    return true;
}

void com::pelagicore::TemperatureService::TemperatureChanged_emitter(double unnamed_arg0)
{
    std::vector<Glib::VariantBase> paramsList;

    paramsList.push_back(Glib::Variant<double >::create((unnamed_arg0)));

    m_connection->emit_signal("/com/pelagicore/TemperatureService",
                              "com.pelagicore.TemperatureService",
                              "TemperatureChanged",
                              Glib::ustring(),
                              Glib::Variant<std::vector<Glib::VariantBase> >::create_tuple(paramsList));
}

void com::pelagicore::TemperatureService::on_bus_acquired(const Glib::RefPtr<Gio::DBus::Connection>& connection,
                         const Glib::ustring& /* name */) {
    Gio::DBus::InterfaceVTable *interface_vtable =
        new Gio::DBus::InterfaceVTable(sigc::mem_fun(this, &TemperatureService::on_method_call),
                                       sigc::mem_fun(this, &TemperatureService::on_interface_get_property),
                                       sigc::mem_fun(this, &TemperatureService::on_interface_set_property));
    try {
        registeredId = connection->register_object(m_objectPath,
        introspection_data->lookup_interface("com.pelagicore.TemperatureService"),
        *interface_vtable);
        m_connection = connection;
    }
    catch(const Glib::Error& ex) {
        g_warning("Registration of object failed");
    }

    return;
}
void com::pelagicore::TemperatureService::on_name_acquired(const Glib::RefPtr<Gio::DBus::Connection>& /* connection */,
                      const Glib::ustring& /* name */) {}

void com::pelagicore::TemperatureService::on_name_lost(const Glib::RefPtr<Gio::DBus::Connection>& /*connection*/,
                  const Glib::ustring& /* name */) {}


bool com::pelagicore::TemperatureService::emitSignal(const std::string& propName,
                                                     Glib::VariantBase& value)
{
    std::map<Glib::ustring, Glib::VariantBase> changedProps;
    std::vector<Glib::ustring> changedPropsNoValue;

    changedProps[propName] = value;

    Glib::Variant<std::map<Glib::ustring, Glib::VariantBase> > changedPropsVar =
            Glib::Variant<std::map <Glib::ustring, Glib::VariantBase> >::create (changedProps);
    Glib::Variant<std::vector<Glib::ustring> > changedPropsNoValueVar =
            Glib::Variant<std::vector<Glib::ustring> >::create(changedPropsNoValue);
    std::vector<Glib::VariantBase> ps;
    ps.push_back(Glib::Variant<Glib::ustring>::create(m_interfaceName));
    ps.push_back(changedPropsVar);
    ps.push_back(changedPropsNoValueVar);
    Glib::VariantContainerBase propertiesChangedVariant = Glib::Variant<std::vector<Glib::VariantBase> >::create_tuple(ps);

    m_connection->emit_signal(m_objectPath,
                              "org.freedesktop.DBus.Properties",
                              "PropertiesChanged",
                              Glib::ustring(),
                              propertiesChangedVariant);

    return true;
}
