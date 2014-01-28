
#include "pelagicontaintodbusadapter.h"

PelagicontainToDBusAdapter::PelagicontainToDBusAdapter(DBus::Connection &connection, Pelagicontain &pelagicontain) :
	DBus::ObjectAdaptor(connection, "/com/pelagicore/Pelagicontain"),
	m_pelagicontain(pelagicontain)
{

}

std::string PelagicontainToDBusAdapter::Echo(const std::string &argument)
{
	return "Hello from Echo";
}

void PelagicontainToDBusAdapter::Launch(const std::string &appId)
{
	m_pelagicontain.launch(appId);
}

void PelagicontainToDBusAdapter::Update(const std::vector<std::string> &config)
{
	m_pelagicontain.update(config);
}

bool PelagicontainToDBusAdapter::Shutdown()
{
	return m_pelagicontain.shutdown();
}
