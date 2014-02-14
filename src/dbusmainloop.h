/*
 *   Copyright (C) 2014 Pelagicore AB
 *   All rights reserved.
 */
#ifndef DBUSMAINLOOP_H
#define DBUSMAINLOOP_H

#include <dbus-c++/dbus.h>

#include "mainloopabstractinterface.h"

/*! A libdbus-c++ specific implementation of the mainloop interface
 */
class DBusMainloop :
	public MainloopAbstractInterface
{
public:
	/*! Constructor
	 *
	 * Takes a pointer to the DBusDispatcher which is used to enter
	 * and leave the mainloop.
	 *
	 * \param dispatcher A pointer to the DBusDispatcher
	 */
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
