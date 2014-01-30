#ifndef PAMINTERFACE_H
#define PAMINTERFACE_H

#include "pamproxy.h"
#include "pamabstractinterface.h"

/*!
 */

class PAMInterface :
	public com::pelagicore::PAM_proxy,
	public DBus::IntrospectableProxy,
	public DBus::ObjectProxy,
	public PAMAbstractInterface
{

public:
	PAMInterface(DBus::Connection &connection);

	/*!
	 * Implements PAMAbstractInterface::registerClient
	 */
	virtual void registerClient(const std::string &cookie,
		const std::string &appId);

	/*!
	 * Implements PAMAbstractInterface::unregisterClient
	 */
	virtual void unregisterClient(const std::string &appId);

	/*!
	 * Implements PAMAbstractInterface::updateFinished
	 */
	virtual void updateFinished();
};

#endif //PAMINTERFACE_H
