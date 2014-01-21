#include <dbus-c++/dbus.h>

#include "dbusadaptor.h"

class ContainedApp : 
	public com::pelagicore::test::ContainedApp_adaptor,
	public DBus::IntrospectableAdaptor,
	public DBus::ObjectAdaptor
{
public:
	ContainedApp (DBus::Connection &conn) : 
		DBus::ObjectAdaptor(conn, "/com/pelagicore/test/ContainedApp")
	{
	
	}
	virtual std::string Echo(const std::string& argument) {
		return argument;
	}

};
int main (int argc, char **argv)
{
	DBus::BusDispatcher dispatcher;

	DBus::default_dispatcher = &dispatcher;
	DBus::Connection bus = DBus::Connection::SessionBus();
	ContainedApp app(bus);

	bus.request_name("com.pelagicore.test.ContainedApp");

	dispatcher.enter();
}
