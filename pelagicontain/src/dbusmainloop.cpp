/*
 *   Copyright (C) 2014 Pelagicore AB
 *   All rights reserved.
 */
#include "dbusmainloop.h"

DBusMainloop::DBusMainloop(DBus::BusDispatcher *dispatcher):
    m_dispatcher(dispatcher)
{
}

DBusMainloop::~DBusMainloop()
{
}

void DBusMainloop::enter()
{
    m_dispatcher->enter();
}

void DBusMainloop::leave()
{
    m_dispatcher->leave();
}
