#!/usr/bin/env python

# Copyright (C) 2016 Pelagicore AB
#
# Permission to use, copy, modify, and/or distribute this software for
# any purpose with or without fee is hereby granted, provided that the
# above copyright notice and this permission notice appear in all copies.
#
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
# WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR
# BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES
# OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
# WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
# ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
# SOFTWARE.
#
# For further information see LICENSE


import gobject
import dbus
import dbus.service
from dbus.mainloop.glib import DBusGMainLoop


class PAMStub(dbus.service.Object):
    register_called = False
    unregisterclient_called = False
    updatefinished_called = False
    BUS_NAME = "com.pelagicore.PAM"

    def __init__(self):
        print "PAM stub started..."
        self.configs = {"":""}
        self.bus = dbus.SessionBus()
        request = self.bus.request_name(self.BUS_NAME, dbus.bus.NAME_FLAG_REPLACE_EXISTING)
        bus_name = dbus.service.BusName(self.BUS_NAME, bus=self.bus)
        dbus.service.Object.__init__(self, bus_name, "/com/pelagicore/PAM")

    @dbus.service.method(BUS_NAME, in_signature="s", out_signature="s", 
        sender_keyword="sender")
    def Echo(self, message, sender=None):
        return message

    @dbus.service.method(BUS_NAME, in_signature="ss", out_signature="", 
        sender_keyword="sender")
    def RegisterClient(self, cookie, appId, sender=None):
        print sender + " called RegisterClient() with args " + "\"" + cookie + "\", \"" + appId + "\""
        self.register_called = True
        self.call_softwarecontainer_update(cookie, self.configs)

    @dbus.service.method(BUS_NAME, in_signature="s", out_signature="",
        sender_keyword="sender")
    def UpdateFinished(self, cookie, sender=None):
        self.updatefinished_called = True
        print sender + " called UpdateFinished()"

    @dbus.service.method(BUS_NAME, in_signature="s", out_signature="",
        sender_keyword="sender")
    def UnregisterClient(self, cookie, sender=None):
        self.unregisterclient_called = True
        print sender + " called UnregisterClient()"

    """ Helper methods available on D-Bus. These does not mirror the actual
        API of PAM.
    """
    @dbus.service.method(BUS_NAME, in_signature="sa{ss}", out_signature="",
        sender_keyword="sender")
    def helper_trigger_update(self, cookie, configs, sender=None):
        self.call_softwarecontainer_update(cookie, configs)

    @dbus.service.method(BUS_NAME, in_signature="a{ss}", out_signature="",
        sender_keyword="sender")
    def helper_set_configs(self, configs, sender=None):
        self.set_configs(configs)

    """ Methods below are used by the component test to verify the expected
        methods have been called on this object by SoftwareContainer
    """
    @dbus.service.method(BUS_NAME, out_signature="b")
    def test_register_called(self):
        return self.register_called

    @dbus.service.method(BUS_NAME, out_signature="b")
    def test_unregisterclient_called(self):
        return self.unregisterclient_called

    @dbus.service.method(BUS_NAME, out_signature="b")
    def test_updatefinished_called(self):
        return self.updatefinished_called

    @dbus.service.method(BUS_NAME)
    def test_reset_values(self):
        self.configs = {"":""}
        self.register_called = False
        self.unregisterclient_called = False
        self.updatefinished_called = False

    """ Non-DBus methods.
    """
    def set_configs(self, configs):
        self.configs = configs

    def call_softwarecontainer_update(self, cookie, configs):
        print "Calling SoftwareContainer::Update with:", configs
        softwarecontainer_remote_object = self.bus.get_object("com.pelagicore.SoftwareContainer" + cookie,
            "/com/pelagicore/SoftwareContainer")
        softwarecontainer_iface = dbus.Interface(softwarecontainer_remote_object,
            "com.pelagicore.SoftwareContainer")
        softwarecontainer_iface.Update(configs)


DBusGMainLoop(set_as_default=True)
myservice = PAMStub()
gobject.MainLoop().run()
