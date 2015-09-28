#!/usr/bin/env python


"""
    Copyright (C) 2014 Pelagicore AB
    All rights reserved.
"""

""" 
    Test that pelagicontain properly kills an app if it refuses to die.
"""

from time import sleep
import pytest
import os
import signal
from subprocess import call
from common import ComponentTestHelper

EVIL_APP = """
#include <signal.h>
#include <unistd.h>
#include <iostream>

void sighandler(int signum)
{
    std::cout << "Got signal " << signum << ", but I don't care!" << std::endl;
}

int main()
{
    signal(SIGINT, sighandler);
    signal(SIGTERM, sighandler);

    int counter = 1;
    while (true) {
        std::cout << "Looping forever, this is loop #" << counter++ << std::endl;
        usleep(1000 * 1000);
    }
}
"""

helper = ComponentTestHelper()

class TestKill(object):
    
    def test_killapp(self, pelagicontain_binary, container_path, teardown_fixture):
        container_before = os.listdir(container_path)
        helper.pam_iface().test_reset_values()
        self.__create_app(container_path)
        sleep(1);

        """ Run pelagicontain and kill the app """
        helper.start_pelagicontain(pelagicontain_binary, container_path)
        pc_pid = helper.pelagicontain_pid()
        pc_iface = helper.pelagicontain_iface()
        pc_iface.Launch(helper.app_id())
        sleep(1) # Let it start
        helper.shutdown_pelagicontain()
        sleep(6) # Timeout in PC is 5 secs
        helper.poke_pelagicontain_zombie()
        sleep(1)

        # If PC has exited it should have cleaned up as well
        assert self.__dir_diff(container_before, container_path)
        # PC should have exited by now, so killing it should yield an error
        exitstatus = call(["kill", "-0", str(pc_pid)])
        assert exitstatus != 0

    def __create_app(self, container_path):
        """ Copy and compile the evil app into the container
            location.
        """
        containedapp_cppfile = container_path + "/com.pelagicore.comptest/bin/containedapp.cpp"
        containedapp_binfile = container_path + "/com.pelagicore.comptest/bin/containedapp"
        with open(containedapp_cppfile, "w") as f:
            print "Overwriting containedapp..."
            f.write(EVIL_APP)
        # Compile
        os.system("g++ -o " + containedapp_binfile + " " + containedapp_cppfile)
        os.system("chmod 755 " + containedapp_binfile)

    def __dir_diff(self, contents_before, path):
        """ Makes sure that there is no diff between
            a previous listdir of the path and now
        """

        contents_after = os.listdir(path)
        diff = list(set(contents_after) - set(contents_before))
        return len(diff) == 0
