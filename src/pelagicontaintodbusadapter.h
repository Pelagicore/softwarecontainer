#include <dbus-c++/dbus.h>

#include "dbusadaptor.h"
#include "pelagicontain.h"

class PelagicontainToDBusAdapter : 
	public com::pelagicore::Pelagicontain_adaptor,
	public DBus::IntrospectableAdaptor,
	public DBus::ObjectAdaptor
{
public:
	PelagicontainToDBusAdapter (DBus::Connection &conn, Pelagicontain &pc);
	virtual std::string Echo(const std::string& argument);
	virtual void Launch(const std::string& appId);

private:
	Pelagicontain m_pelagicontain;
};
