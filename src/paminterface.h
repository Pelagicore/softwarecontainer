#ifndef PAMINTERFACE_H
#define PAMINTERFACE_H

#include "pamproxy.h"

/*!
 */

class PAMInterface :
	public com::pelagicore::PAM_proxy,
	public DBus::IntrospectableProxy,
	public DBus::ObjectProxy
{

public:
	PAMInterface(DBus::Connection &connection):
		DBus::ObjectProxy(connection, "/com/pelagicore/PAM",
			"com.pelagicore.PAM")
	{
	}
	
};

#endif //PAMINTERFACE_H
