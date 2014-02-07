/*
 *   Copyright (C) 2014 Pelagicore AB
 *   All rights reserved.
 */
#ifndef PAMABSTRACTINTERFACE_H
#define PAMABSTRACTINTERFACE_H

#include <string>

class PAMAbstractInterface {

public:
	virtual void registerClient(const std::string &cookie,
		const std::string &appId) = 0;
	virtual void unregisterClient(const std::string &appId) = 0;
	virtual void updateFinished(const std::string &appId) = 0;
};

#endif /* PAMABSTRACTINTERFACE_H */
