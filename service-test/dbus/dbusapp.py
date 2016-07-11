#!/usr/bin/env python

""" copyright Pelagicore AB 2016 """

import os
from gi.repository import GObject
import dbus
import dbus.service
import pydbus
import threading
import time
import argparse


from dbus.mainloop.glib import DBusGMainLoop
DBusGMainLoop(set_as_default=True)
BUS_NAME = "com.service.TestService"
OPATH = "/Object"
IFACE = "com.service.TestInterface"

NR_OF_REQUESTS = 1000

class Service(dbus.service.Object):
    def __init__(self, bus):
        self.__bus = bus
        name = dbus.service.BusName(BUS_NAME, bus=self.__bus)
        self.requests = 0
        dbus.service.Object.__init__(self, name, OPATH)

    @dbus.service.method(IFACE, in_signature="s", out_signature="s")
    def Bounce(self, message):
        self.requests += 1
        return message


class Server(threading.Thread):
    def __init__(self):
        threading.Thread.__init__(self)
        self.__loop = GObject.MainLoop()
        self.service = None

    def run(self):
        connection = os.environ.get("DBUS_SESSION_BUS_ADDRESS")
        bus = dbus.bus.BusConnection(connection)
        self.service = Service(bus)
        self.__loop.run()

    def wait_until_requests(self, timeout=1):
        if self.service is None:
            print("Server not started yet, aborting wait...")
            return False

        return wait_until(lambda : self.service.requests == NR_OF_REQUESTS, timeout)

    def terminate(self):
        self.__loop.quit()

def wait_until(somepredicate, timeout, period=0.25, *args, **kwargs):
    mustend = time.time() + timeout
    while time.time() < mustend:
        if somepredicate(*args, **kwargs): return True
        time.sleep(period)
    return False

class Client():

    def __init__(self):
        self.bus = pydbus.SessionBus()
        self.good_resp = 0

    def run(self):
        self.good_resp = 0
        inp = "DBusTestMessage"
        remote_object = self.bus.get(
            BUS_NAME, OPATH
        )

        for _ in range(0, NR_OF_REQUESTS):
            ans = remote_object.Bounce(inp)
            if inp == ans:
                self.good_resp += 1

    def check_all_good_resp(self):
        return self.good_resp == NR_OF_REQUESTS

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Process some integers.')
    parser.add_argument('mode', choices=["client", "server"],
                    help='Run the dbusapp as "server" or "client"')

    args = parser.parse_args()
    if args.mode == "server":
        r = Server()
        r.start()
    elif args.mode == "client":
        c = Client()
        c.run()
