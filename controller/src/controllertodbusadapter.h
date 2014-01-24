
#include <dbus-c++/dbus.h>

#include "dbusadaptor.h"
#include "controller.h"

class ControllerToDBusAdapter : 
	public com::pelagicore::Controller_adaptor,
	public DBus::IntrospectableAdaptor,
	public DBus::ObjectAdaptor
{
public:
	ControllerToDBusAdapter(DBus::Connection &conn, Controller &controller);
	virtual std::string Echo(const std::string& argument);

private:
	Controller m_controller;
};
