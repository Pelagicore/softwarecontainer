/*
 *   Copyright (C) 2014 Pelagicore AB
 *   All rights reserved.
 */
#include "dbusmainloop.h"

DBusMainloop::DBusMainloop(Glib::RefPtr<Glib::MainLoop> ml):
    m_dispatcher(ml)
{
}

DBusMainloop::~DBusMainloop()
{
}

void DBusMainloop::leave()
{
    m_dispatcher->quit();
}
