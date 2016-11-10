
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

import sys
import os

VERBOSE = True


def LOG(message):
    """ To avoid spammy output when needed we wrap the printouts
    """
    if VERBOSE is True:
        print message


class Helper(object):
    """ The helper is run on the inside of containers to investigate
        and interact with from within.

        This means that paths etc. are relative to how the container is
        configured, e.g. when creating it and what gateway configs are used.
    """

    def __init__(self, file_base_path):
        """ file_base_path needs to make sense inside the container
        """
        self.__base_path = file_base_path

    def ping(self, host):
        LOG("pings" + host)
        is_pingable = os.system("ping -c1 " + host) 
        return is_pingable;

    def write_result(self, data):
        with open(self.__base_path + "/ping_result", "w") as fh:
            fh.write(str(data))
        LOG(data)

    def ifconfig(self, file_name):
        os.system("ip -4 addr show eth0 | grep inet | awk '{print $2}' >> " + file_name)
        

def compare_ips(base_path, file1, file2):
    with open(base_path + "/" + file1, "r") as fh:
        ip1 = fh.readline()
     
    with open(base_path + "/" + file2, "r") as fh:
        ip2 = fh.readline()
 
    return (ip1==ip2)


def ping_result(base_path):
    with open(base_path + "/ping_result", "r") as fh:
        is_pingable = fh.readline()
    LOG(is_pingable)
    return is_pingable;

def remove_file(base_path, filename):
    os.remove(base_path + "/" + filename)


if __name__ == "__main__":
    file_base_path = sys.argv[1]

    h = Helper(file_base_path)

    if sys.argv[2] == "--ping":    
        host = sys.argv[3]
        LOG(host)
        is_pingable = h.ping(host)
        h.write_result(is_pingable);
    elif sys.argv[2] == "--ifconfig":
        file_name = sys.argv[3]
        h.ifconfig(file_name)
