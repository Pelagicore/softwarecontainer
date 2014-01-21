#include <dbus-c++/dbus.h>

#include "dbusadaptor.h"

class DBusTestApp : 
	public com::pelagicore::test::pelagicontaintestapp_adaptor,
	public DBus::IntrospectableAdaptor,
	public DBus::ObjectAdaptor
{
public:
	DBusTestApp (DBus::Connection &conn) : 
		DBus::ObjectAdaptor(conn, "/com/pelagicore/test/pelagicontaintestapp")
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
	DBusTestApp testapp(bus);

	bus.request_name("com.pelagicore.test.pelagicontaintestapp");

	dispatcher.enter();
}
