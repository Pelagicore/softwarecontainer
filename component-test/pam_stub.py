#!/usr/bin/env python

import gobject
import dbus
import dbus.service
from dbus.mainloop.glib import DBusGMainLoop

class PAMStub(dbus.service.Object):
    register_called = False
    unregister_called = False
    updatefinished_called = False
    BUS_NAME="com.pelagicore.PAM"
    
    def __init__(self):
        bus = dbus.SessionBus()
        request = bus.request_name(self.BUS_NAME, dbus.bus.NAME_FLAG_REPLACE_EXISTING)
        bus_name = dbus.service.BusName(self.BUS_NAME, bus=bus)
        dbus.service.Object.__init__(self, bus_name, '/com/pelagicore/PAM')

    @dbus.service.method(BUS_NAME, in_signature='ss', out_signature='')
    def Register(self, app, gwId):
        self.register_called = True
        print "ATTENTION!! Now update should be called, but PAM is a stub!"

    @dbus.service.method(BUS_NAME, in_signature='s', out_signature='')
    def Unregister(self, appId):
        self.unregister_called = True

    @dbus.service.method(BUS_NAME, in_signature='s', out_signature='')
    def UpdateFinished(self, appId):
        self.updatefinished_called = True

            
    """ Methods below are used by the component test to verify the expected
        methods have been called on this object by Pelagicontain
    """
    @dbus.service.method(BUS_NAME, out_signature='b')
    def test_register_called(self):
        return self.register_called

    @dbus.service.method(BUS_NAME, out_signature='b')
    def test_unregister_called(self):
        return self.unregister_called

    @dbus.service.method(BUS_NAME, out_signature='b')
    def test_updatefinished_called(self):
        return self.updatefinished_called

    @dbus.service.method(BUS_NAME)
    def test_reset_values(self):
        self.register_called = False
        self.unregister_called = False
        self.updatefinished_called = False

DBusGMainLoop(set_as_default=True)
myservice = PAMStub()
gobject.MainLoop().run()
