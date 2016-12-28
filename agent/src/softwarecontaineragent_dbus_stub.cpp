#include "softwarecontaineragent_dbus_stub.h"

Glib::ustring interfaceXml0 = R"XML_DELIMITER(
<?xml version="1.0" encoding="UTF-8" ?>
<node name="/com/pelagicore/SoftwareContainer">
    <interface name="com.pelagicore.SoftwareContainerAgent">
        <method name="List">
            <arg direction="out" type="ai" name="containers" />
        </method>

        <method name="Create">
            <arg direction="in" type="s" name="config" />
            <arg direction="out" type="i" name="containerID" />
            <arg direction="out" type="b" name="success" />
        </method>

        <method name="Execute">
            <arg direction="in" type="i" name="containerID" />
            <arg direction="in" type="s" name="commandLine" />
            <arg direction="in" type="s" name="workingDirectory" />
            <arg direction="in" type="s" name="outputFile" />
            <arg direction="in" type="a{ss}" name="env" />
            <arg direction="out" type="i" name="pid" />
            <arg direction="out" type="b" name="success" />
        </method>

        <method name="Suspend">
            <arg direction="in" type="i" name="containerID" />
            <arg direction="out" type="b" name="success" />
        </method>

        <method name="Resume">
            <arg direction="in" type="i" name="containerID" />
            <arg direction="out" type="b" name="success" />
        </method>

        <method name="Destroy">
            <arg direction="in" type="i" name="containerID" />
            <arg direction="out" type="b" name="success" />
        </method>

        <method name="BindMount">
            <arg direction="in" type="i" name="containerID" />
            <arg direction="in" type="s" name="pathInHost" />
            <arg direction="in" type="s" name="pathInContainer" />
            <arg direction="in" type="b" name="readOnly" />
            <arg direction="out" type="b" name="success" />
        </method>

        <method name="ListCapabilities">
            <arg direction="out" type="as" name="capabilities" />
        </method>

        <method name="SetCapabilities">
            <arg direction="in" type="i" name="containerID" />
            <arg direction="in" type="as" name="capabilities" />
            <arg direction="out" type="b" name="success" />
        </method>

        <signal name="ProcessStateChanged">
            <arg direction="out" type="i" name="containerID"/>
            <arg direction="out" type="u" name="processID"/>
            <arg direction="out" type="b" name="isRunning"/>
            <arg direction="out" type="u" name="exitCode"/>
        </signal>

    </interface>
</node>
)XML_DELIMITER";

com::pelagicore::SoftwareContainerAgent::SoftwareContainerAgent() : connectionId(0), registeredId(0), m_objectPath("/com/pelagicore/SoftwareContainerAgent"), m_interfaceName("com.pelagicore.SoftwareContainerAgent") {

    ProcessStateChanged_signal.connect(sigc::mem_fun(this, &SoftwareContainerAgent::ProcessStateChanged_emitter));

}
void com::pelagicore::SoftwareContainerAgent::connect (
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
                                       sigc::mem_fun(this, &SoftwareContainerAgent::on_bus_acquired),
                                       sigc::mem_fun(this, &SoftwareContainerAgent::on_name_acquired),
                                       sigc::mem_fun(this, &SoftwareContainerAgent::on_name_lost));
}

void com::pelagicore::SoftwareContainerAgent::on_method_call(const Glib::RefPtr<Gio::DBus::Connection>& /* connection */,
                   const Glib::ustring& /* sender */,
                   const Glib::ustring& /* object_path */,
                   const Glib::ustring& /* interface_name */,
                   const Glib::ustring& method_name,
                   const Glib::VariantContainerBase& parameters,
                   const Glib::RefPtr<Gio::DBus::MethodInvocation>& invocation)
{

    if (method_name.compare("List") == 0) {
        List(
            SoftwareContainerAgentMessageHelper(invocation));
    }
    if (method_name.compare("Create") == 0) {
        Glib::Variant<Glib::ustring > base_config;
        parameters.get_child(base_config, 0);
        Glib::ustring p_config;
        p_config = base_config.get();

        Create(
            Glib::ustring(p_config),
            SoftwareContainerAgentMessageHelper(invocation));
    }
    if (method_name.compare("Execute") == 0) {
        Glib::Variant<gint32 > base_containerID;
        parameters.get_child(base_containerID, 0);
        gint32 p_containerID;
        p_containerID = base_containerID.get();

        Glib::Variant<Glib::ustring > base_commandLine;
        parameters.get_child(base_commandLine, 1);
        Glib::ustring p_commandLine;
        p_commandLine = base_commandLine.get();

        Glib::Variant<Glib::ustring > base_workingDirectory;
        parameters.get_child(base_workingDirectory, 2);
        Glib::ustring p_workingDirectory;
        p_workingDirectory = base_workingDirectory.get();

        Glib::Variant<Glib::ustring > base_outputFile;
        parameters.get_child(base_outputFile, 3);
        Glib::ustring p_outputFile;
        p_outputFile = base_outputFile.get();

        Glib::Variant<std::map<Glib::ustring, Glib::ustring> > base_env;
        parameters.get_child(base_env, 4);
        std::map<Glib::ustring, Glib::ustring> p_env;
        p_env = base_env.get();

        Execute(
            (p_containerID),
            Glib::ustring(p_commandLine),
            Glib::ustring(p_workingDirectory),
            Glib::ustring(p_outputFile),
            SoftwareContainerAgentCommon::glibStringMapToStdStringMap(p_env),
            SoftwareContainerAgentMessageHelper(invocation));
    }
    if (method_name.compare("Suspend") == 0) {
        Glib::Variant<gint32 > base_containerID;
        parameters.get_child(base_containerID, 0);
        gint32 p_containerID;
        p_containerID = base_containerID.get();

        Suspend(
            (p_containerID),
            SoftwareContainerAgentMessageHelper(invocation));
    }
    if (method_name.compare("Resume") == 0) {
        Glib::Variant<gint32 > base_containerID;
        parameters.get_child(base_containerID, 0);
        gint32 p_containerID;
        p_containerID = base_containerID.get();

        Resume(
            (p_containerID),
            SoftwareContainerAgentMessageHelper(invocation));
    }
    if (method_name.compare("Destroy") == 0) {
        Glib::Variant<gint32 > base_containerID;
        parameters.get_child(base_containerID, 0);
        gint32 p_containerID;
        p_containerID = base_containerID.get();

        Destroy(
            (p_containerID),
            SoftwareContainerAgentMessageHelper(invocation));
    }
    if (method_name.compare("BindMount") == 0) {
        Glib::Variant<gint32 > base_containerID;
        parameters.get_child(base_containerID, 0);
        gint32 p_containerID;
        p_containerID = base_containerID.get();

        Glib::Variant<Glib::ustring > base_pathInHost;
        parameters.get_child(base_pathInHost, 1);
        Glib::ustring p_pathInHost;
        p_pathInHost = base_pathInHost.get();

        Glib::Variant<Glib::ustring > base_pathInContainer;
        parameters.get_child(base_pathInContainer, 2);
        Glib::ustring p_pathInContainer;
        p_pathInContainer = base_pathInContainer.get();

        Glib::Variant<bool > base_readOnly;
        parameters.get_child(base_readOnly, 3);
        bool p_readOnly;
        p_readOnly = base_readOnly.get();

        BindMount(
            (p_containerID),
            Glib::ustring(p_pathInHost),
            Glib::ustring(p_pathInContainer),
            (p_readOnly),
            SoftwareContainerAgentMessageHelper(invocation));
    }
    if (method_name.compare("ListCapabilities") == 0) {
        ListCapabilities(
            SoftwareContainerAgentMessageHelper(invocation));
    }
    if (method_name.compare("SetCapabilities") == 0) {
        Glib::Variant<gint32 > base_containerID;
        parameters.get_child(base_containerID, 0);
        gint32 p_containerID;
        p_containerID = base_containerID.get();

        Glib::Variant<std::vector<Glib::ustring> > base_capabilities;
        parameters.get_child(base_capabilities, 1);
        std::vector<Glib::ustring> p_capabilities;
        p_capabilities = base_capabilities.get();

        SetCapabilities(
            (p_containerID),
            SoftwareContainerAgentCommon::glibStringVecToStdStringVec(p_capabilities),
            SoftwareContainerAgentMessageHelper(invocation));
    }
    }

void com::pelagicore::SoftwareContainerAgent::on_interface_get_property(Glib::VariantBase& property,
                      const Glib::RefPtr<Gio::DBus::Connection>& connection,
                      const Glib::ustring& sender,
                      const Glib::ustring& object_path,
                      const Glib::ustring& interface_name,
                      const Glib::ustring& property_name) {

}

bool com::pelagicore::SoftwareContainerAgent::on_interface_set_property(
       const Glib::RefPtr<Gio::DBus::Connection>& connection,
       const Glib::ustring& sender,
       const Glib::ustring& object_path,
       const Glib::ustring& interface_name,
       const Glib::ustring& property_name,
       const Glib::VariantBase& value) {


    return true;
}

void com::pelagicore::SoftwareContainerAgent::ProcessStateChanged_emitter(gint32 containerID, guint32 processID, bool isRunning, guint32 exitCode) {
            std::vector<Glib::VariantBase> paramsList;

paramsList.push_back(Glib::Variant<gint32 >::create((containerID)));;


paramsList.push_back(Glib::Variant<guint32 >::create((processID)));;


paramsList.push_back(Glib::Variant<bool >::create((isRunning)));;


paramsList.push_back(Glib::Variant<guint32 >::create((exitCode)));;

m_connection->emit_signal(
              "/com/pelagicore/SoftwareContainerAgent",
              "com.pelagicore.SoftwareContainerAgent",
              "ProcessStateChanged",
              Glib::ustring(),
              Glib::Variant<std::vector<Glib::VariantBase> >::create_tuple(paramsList));
      }

void com::pelagicore::SoftwareContainerAgent::on_bus_acquired(const Glib::RefPtr<Gio::DBus::Connection>& connection,
                         const Glib::ustring& /* name */) {
    Gio::DBus::InterfaceVTable *interface_vtable =
          new Gio::DBus::InterfaceVTable(
                sigc::mem_fun(this, &SoftwareContainerAgent::on_method_call),
                sigc::mem_fun(this, &SoftwareContainerAgent::on_interface_get_property),
                sigc::mem_fun(this, &SoftwareContainerAgent::on_interface_set_property));
    try {
        registeredId = connection->register_object(m_objectPath,
            introspection_data->lookup_interface("com.pelagicore.SoftwareContainerAgent"),
            *interface_vtable);
        m_connection = connection;
    }
    catch(const Glib::Error& ex) {
        g_warning("Registration of object failed");
    }

    return;
}
void com::pelagicore::SoftwareContainerAgent::on_name_acquired(const Glib::RefPtr<Gio::DBus::Connection>& /* connection */,
                      const Glib::ustring& /* name */) {}

void com::pelagicore::SoftwareContainerAgent::on_name_lost(const Glib::RefPtr<Gio::DBus::Connection>& connection,
                  const Glib::ustring& /* name */) {}


bool com::pelagicore::SoftwareContainerAgent::emitSignal(const std::string& propName, Glib::VariantBase& value) {
    std::map<Glib::ustring, Glib::VariantBase> changedProps;
    std::vector<Glib::ustring> changedPropsNoValue;

    changedProps[propName] = value;

    Glib::Variant<std::map<Glib::ustring,  Glib::VariantBase> > changedPropsVar = Glib::Variant<std::map <Glib::ustring, Glib::VariantBase> >::create (changedProps);
    Glib::Variant<std::vector<Glib::ustring> > changedPropsNoValueVar = Glib::Variant<std::vector<Glib::ustring> >::create(changedPropsNoValue);
    std::vector<Glib::VariantBase> ps;
    ps.push_back(Glib::Variant<Glib::ustring>::create(m_interfaceName));
    ps.push_back(changedPropsVar);
    ps.push_back(changedPropsNoValueVar);
    Glib::VariantContainerBase propertiesChangedVariant = Glib::Variant<std::vector<Glib::VariantBase> >::create_tuple(ps);

    m_connection->emit_signal(
        m_objectPath,
        "org.freedesktop.DBus.Properties",
        "PropertiesChanged",
        Glib::ustring(),
        propertiesChangedVariant);

    return true;
}
