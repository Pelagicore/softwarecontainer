#pragma once
#include <string>
#include <glibmm.h>
#include <giomm.h>

#include "softwarecontaineragent_dbus_common.h"


namespace com {
namespace pelagicore {
class SoftwareContainerAgent {
public:
    SoftwareContainerAgent ();

protected:
    void connect (Gio::DBus::BusType, std::string);

    virtual void List (
        const SoftwareContainerAgentMessageHelper msg) = 0;

    virtual void Create (
        std::string config,
        const SoftwareContainerAgentMessageHelper msg) = 0;

    virtual void Execute (
        gint32 containerID,
        std::string commandLine,
        std::string workingDirectory,
        std::string outputFile,
        std::map<std::string, std::string>  env,
        const SoftwareContainerAgentMessageHelper msg) = 0;

    virtual void Suspend (
        gint32 containerID,
        const SoftwareContainerAgentMessageHelper msg) = 0;

    virtual void Resume (
        gint32 containerID,
        const SoftwareContainerAgentMessageHelper msg) = 0;

    virtual void Destroy (
        gint32 containerID,
        const SoftwareContainerAgentMessageHelper msg) = 0;

    virtual void BindMount (
        gint32 containerID,
        std::string pathInHost,
        std::string pathInContainer,
        bool readOnly,
        const SoftwareContainerAgentMessageHelper msg) = 0;

    virtual void ListCapabilities (
        const SoftwareContainerAgentMessageHelper msg) = 0;

    virtual void SetCapabilities (
        gint32 containerID,
        std::vector<std::string>  capabilities,
        const SoftwareContainerAgentMessageHelper msg) = 0;

    void ProcessStateChanged_emitter(gint32, guint32, bool, guint32);
    sigc::signal<void, gint32, guint32, bool, guint32 > ProcessStateChanged_signal;

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

    /*
     * We emit these signals when we have a lost/acquired a name on dbus,
     * so that subclasses of this class can bind slots to these signals.
     */
    sigc::signal<void, std::string> name_lost;
    sigc::signal<void, std::string> name_acquired;
    sigc::signal<void, std::string> object_not_registered;

private:
    bool emitSignal(const std::string& propName, Glib::VariantBase& value);

    guint connectionId;
    guint registeredId;
    Glib::RefPtr<Gio::DBus::NodeInfo> introspection_data;
    Glib::RefPtr<Gio::DBus::Connection> m_connection;
    std::string m_objectPath;
    std::string m_interfaceName;
};
}// pelagicore
}// com

