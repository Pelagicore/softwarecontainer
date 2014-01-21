
#include "pelagicontaintodbusadapter.h"

PelagicontainToDBusAdapter::PelagicontainToDBusAdapter(DBus::Connection &connection, Pelagicontain &pelagicontain) :
	DBus::ObjectAdaptor(connection, "/com/pelagicore/Pelagicontain"),
	m_pelagicontain(pelagicontain)
{
	
}

std::string PelagicontainToDBusAdapter::Echo(const std::string& argument) {
	return "Hello from Echo";
}

void PelagicontainToDBusAdapter::Launch(const std::string& appId) {
	// We should call PAM::register(appId, gwId) here.
}

void PelagicontainToDBusAdapter::Update(const std::vector<std::string> &config) {
	// PAM will call this method when we have called PAM::Register()
}
