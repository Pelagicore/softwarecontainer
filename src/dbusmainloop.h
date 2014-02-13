/*
 *   Copyright (C) 2014 Pelagicore AB
 *   All rights reserved.
 */
#ifndef DBUSMAINLOOP_H
#define DBUSMAINLOOP_H

#include <dbus-c++/dbus.h>

#include "mainloopabstractinterface.h"

class DBusMainloop :
	public MainloopAbstractInterface
{
public:
	DBusMainloop(DBus::BusDispatcher *dispatcher);
	~DBusMainloop();

	/*! Implements MainloopAbstractInterface::enter
	 */
	virtual void enter();

	/*! Implements MainloopAbstractInterface::leave
	 */
	virtual void leave();

private:
	DBus::BusDispatcher *m_dispatcher;
};

#endif /* DBUSMAINLOOP_H */
