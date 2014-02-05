
#include "paminterface.h"

PAMInterface::PAMInterface(DBus::Connection &connection):
	DBus::ObjectProxy(connection, "/com/pelagicore/PAM", "com.pelagicore.PAM")
{
}

void PAMInterface::registerClient(const std::string &cookie,
	const std::string &appId)
{
	RegisterClient(cookie, appId);
}

void PAMInterface::unregisterClient(const std::string &appId)
{
	UnregisterClient(appId);
}

void PAMInterface::updateFinished()
{
	UpdateFinished();
}
