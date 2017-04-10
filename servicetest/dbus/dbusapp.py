#!/usr/bin/env python

# Copyright (C) 2016-2017 Pelagicore AB
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


import os
from gi.repository import GObject
import dbus
import dbus.service
import pydbus
import threading
import time
import argparse
import random
import operator


from dbus.mainloop.glib import DBusGMainLoop
DBusGMainLoop(set_as_default=True)
BUS_NAME = "com.service.TestService"
OPATH = "/Object"
IFACE = "com.service.TestInterface"

NR_OF_REQUESTS = 1000
CLIENT_MESSAGE_SIZE = 256


class Service(dbus.service.Object):

    def __init__(self, bus, outdir):
        self.__outdir = outdir
        self.__bus = bus
        name = dbus.service.BusName(BUS_NAME, bus=self.__bus)
        self.requests = 0
        dbus.service.Object.__init__(self, name, OPATH)
        # Create file so tests can assume there is something there even
        # if it's supposed to be empty
        with open(self.__outdir + "/service_output", "w") as fh:
            fh.write("")

    @dbus.service.method(IFACE, in_signature="s", out_signature="s")
    def Bounce(self, message):
        self.requests += 1
        return message

    @dbus.service.method(IFACE, in_signature="s", out_signature="s")
    def Ping(self, message):
        print "Got a Ping: ", message
        with open(self.__outdir + "/service_output", "w") as fh:
            fh.write(message)
        return message


class Server(threading.Thread):

    def __init__(self, outdir):
        self.__outdir = outdir
        threading.Thread.__init__(self)
        self.__loop = GObject.MainLoop()
        self.service = None

    def run(self):
        connection = os.environ.get("DBUS_SESSION_BUS_ADDRESS")
        bus = dbus.bus.BusConnection(connection)
        self.service = Service(bus, self.__outdir)
        self.__loop.run()

    def wait_until_requests(self, multiplier=1, timeout=1):
        if self.service is None:
            print("Server not started yet, aborting wait...")
            return False
        expected_requests = NR_OF_REQUESTS * multiplier
        ans = wait_until(lambda: self.service.requests == expected_requests, timeout)
        return ans

    def terminate(self):
        self.service.remove_from_connection()
        self.service = None
        if self.__loop is not None:
            self.__loop.quit()
            self.__loop = None


def wait_until(somepredicate, timeout, period=0.25, *args, **kwargs):
    mustend = time.time() + timeout
    while time.time() < mustend:
        if somepredicate(*args, **kwargs):
            return True
        time.sleep(period)
    return False


class Client():

    def __init__(self, message_size=CLIENT_MESSAGE_SIZE):
        self.bus = pydbus.SessionBus()
        self.good_resp = 0
        self.message_size = message_size
        self.method = None
        self.remote_object = None

    def run(self, method=None):
        self.method = method
        self.remote_object = self.bus.get(BUS_NAME, OPATH)
        if self.method == "Ping":
            self.call_ping()
        else:
            self.good_resp = 0
            alphab = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"
            inp = reduce(operator.add, [random.choice(alphab) for x in range(0, self.message_size - 37)])

            for _ in range(0, NR_OF_REQUESTS):
                ans = self.remote_object.Bounce(inp)
                if inp == ans:
                    self.good_resp += 1

    def check_all_good_resp(self):
        return self.good_resp == NR_OF_REQUESTS

    def call_ping(self):
        print "Will call Ping"
        self.remote_object.Ping("Hello")


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Process some integers.')
    parser.add_argument('mode', choices=["client", "server"],
                        help='Run the dbusapp as "server" or "client"')
    parser.add_argument('--size', type=int, default=CLIENT_MESSAGE_SIZE,
                        help='Size of the messages sent by client')
    parser.add_argument('--method', type=str, default=None,
                        help='Method to call on service')
    parser.add_argument('--outdir', type=str, default="testoutput/",
                        help='Directory where to put output from service')

    args = parser.parse_args()
    if args.mode == "server":
        r = Server(args.outdir)
        r.start()
    elif args.mode == "client":
        print "Using mode 'client'"
        print "method: ", args.method
        c = Client()
        c.run(method=args.method)
