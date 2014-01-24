
#include "controllertodbusadapter.h"

ControllerToDBusAdapter::ControllerToDBusAdapter(DBus::Connection &connection, Controller &controller) :
	DBus::ObjectAdaptor(connection, "/com/pelagicore/Controller"),
	m_controller(controller)
{

}

std::string ControllerToDBusAdapter::Echo(const std::string &argument) {
	return "Hello from Echo";
}
