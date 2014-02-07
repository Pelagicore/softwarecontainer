/*
 *   Copyright (C) 2014 Pelagicore AB
 *   All rights reserved.
 */
#include "pelagicontaintodbusadapter.h"

PelagicontainToDBusAdapter::PelagicontainToDBusAdapter(DBus::Connection &connection,
		const std::string &objPath, Pelagicontain &pelagicontain) :
	DBus::ObjectAdaptor(connection, objPath),
	m_pelagicontain(&pelagicontain)
{

}

std::string PelagicontainToDBusAdapter::Echo(const std::string &argument)
{
	return "Hello from Echo";
}

void PelagicontainToDBusAdapter::Launch(const std::string &appId)
{
	m_pelagicontain->launch(appId);
}

void PelagicontainToDBusAdapter::Update(const std::map<std::string, std::string> &configs)

{
	m_pelagicontain->update(configs);
}

void PelagicontainToDBusAdapter::Shutdown()
{
	m_pelagicontain->shutdown();
}
