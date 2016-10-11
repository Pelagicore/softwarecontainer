
/*
 * Copyright (C) 2016 Pelagicore AB
 *
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
 * WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR
 * BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES
 * OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
 * ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 *
 * For further information see LICENSE
 */

#include "softwarecontainer_test.h"
#include "functionjob.h"
#include <softwarecontainer-common.h>
#include <netlink.h>

#include <linux/if_arp.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

/*
 * A class to test the netlink code
 *
 * Note: most of the test cases uses FunctionJob to run stuff inside a SoftwareContainer instance,
 * but not all. The reason for running inside a container instance is that these mess with the
 * network settings of the system it runs on, so one wouldn't want to run anything other than
 * read-only stuff on one's developer machine.
 *
 * Those tests that run in the host are marked as such with a comment
 */

class NetlinkTest : public SoftwareContainerTest
{
public:
    NetlinkTest() { }
    int ERROR = 1;
    int SUCCESS = 0;
    const char *IFACE = "eth0";
    const char *IFACEADDR = "1.2.3.4";
    const char *GWADDR = "1.2.3.1";
    static constexpr int NETMASK = 24;
};

// Note, running in host
TEST_F(NetlinkTest, DumpOK) {
    Netlink netlink;
    ASSERT_TRUE(isSuccess(netlink.checkKernelDump()));
}

TEST_F(NetlinkTest, LinkUpDown) {

    // Bring the link up (fail if it is already up)
    FunctionJob jobLinkUp(*sc, [this] () {
        Netlink netlink;

        // Get eth0 status
        Netlink::LinkInfo linkDown;
        if (isError(netlink.findLink(IFACE, linkDown))) {
            return ERROR;
        }
        int linkIndex = linkDown.first.ifi_index;

        // Link was already up
        if ((linkDown.first.ifi_flags & IFF_UP) != 0) {
            return ERROR;
        }

        // Bring the link up
        if (isError(netlink.linkUp(linkIndex))) {
            return ERROR;
        }

        return SUCCESS;
    });
    jobLinkUp.start();
    ASSERT_TRUE(jobLinkUp.wait() == SUCCESS);

    // Check that the link is up
    FunctionJob jobCheckLinkUp(*sc, [this] () {
        Netlink netlink;

        // Check that link was up
        Netlink::LinkInfo linkUp;
        if (isError(netlink.findLink(IFACE, linkUp))) {
            return ERROR;
        }

        // Link is down :(
        if ((linkUp.first.ifi_flags & IFF_UP) == 0) {
            return ERROR;
        }

        return SUCCESS;
    });
    jobCheckLinkUp.start();
    ASSERT_TRUE(jobCheckLinkUp.wait() == SUCCESS);

    // Bring the link down
    FunctionJob jobLinkDown(*sc, [this] () {
        Netlink netlink;
         // Get eth0 status
        Netlink::LinkInfo linkUp;
        if (isError(netlink.findLink(IFACE, linkUp))) {
            return ERROR;
        }
        int linkIndex = linkUp.first.ifi_index;

        // Link was already down
        if ((linkUp.first.ifi_flags & IFF_UP) == 0) {
            return ERROR;
        }

        // Bring the link up
        if (isError(netlink.linkDown(linkIndex))) {
            return ERROR;
        }

        return SUCCESS;
    });
    jobLinkDown.start();
    ASSERT_TRUE(jobLinkDown.wait() == SUCCESS);

    // Check that the link is actually down
    FunctionJob jobCheckLinkDown(*sc, [this] () {
        Netlink netlink;

        // Check that link was up
        Netlink::LinkInfo linkDown;
        if (isError(netlink.findLink(IFACE, linkDown))) {
            return ERROR;
        }

        // Link is up :(
        if ((linkDown.first.ifi_flags & IFF_UP) != 0) {
            return ERROR;
        }

        return SUCCESS;
    });
    jobCheckLinkDown.start();
    ASSERT_TRUE(jobCheckLinkDown.wait() == SUCCESS);
}

/*
 * Make sure setting an IP address succeeds
 */
TEST_F(NetlinkTest, SetIP) {
    FunctionJob checkIPNotSet(*sc, [this] () {
        Netlink netlink;

        // Get link index
        Netlink::LinkInfo link;
        if (isError(netlink.findLink(IFACE, link))) {
            return ERROR;
        }
        int ifaceIndex = link.first.ifi_index;

        // Get addresses
        std::vector<Netlink::AddressInfo> addresses;
        if (isError(netlink.findAddresses(ifaceIndex, addresses))) {
            return ERROR;
        }

        // IP should not be set before we've set it
        if (isSuccess(netlink.hasAddress(addresses, AF_INET, IFACEADDR))) {
            return ERROR;
        }

        return SUCCESS;
    });
    checkIPNotSet.start();
    ASSERT_TRUE(checkIPNotSet.wait() == SUCCESS);

    FunctionJob jobSetIP(*sc, [this] () {
        Netlink netlink;
        Netlink::LinkInfo link;
        if (isError(netlink.findLink(IFACE, link))) {
            return ERROR;
        }
        int ifaceIndex = link.first.ifi_index;

        in_addr ip;
        if (inet_pton(AF_INET, IFACEADDR, &ip) != 1) {
            return ERROR;
        }

        if (isError(netlink.setIP(ifaceIndex, ip, NETMASK))) {
            return ERROR;
        }

        return SUCCESS;
    });
    jobSetIP.start();
    ASSERT_TRUE(jobSetIP.wait() == SUCCESS);

    FunctionJob checkIPSet(*sc, [this] () {
        Netlink netlink;

        // Get link index
        Netlink::LinkInfo link;
        if (isError(netlink.findLink(IFACE, link))) {
            return ERROR;
        }
        int ifaceIndex = link.first.ifi_index;

        // Get addresses
        std::vector<Netlink::AddressInfo> addresses;
        if (isError(netlink.findAddresses(ifaceIndex, addresses))) {
            return ERROR;
        }

        if (isError(netlink.hasAddress(addresses, AF_INET, IFACEADDR))) {
            return ERROR;
        }

        return SUCCESS;
    });
    checkIPSet.start();
    ASSERT_TRUE(checkIPSet.wait() == SUCCESS);
}

/*
 * Make sure setting the gateway fails when network link is down.
 * Kernel will respond with: network is unreachable.
 */
TEST_F(NetlinkTest, SetGatewayWithoutUp) {
    // Set the gateway
    FunctionJob jobSetGateway(*sc, [this] () {
        Netlink netlink;
        if (isError(netlink.setDefaultGateway(GWADDR))) {
            return ERROR;
        } else {
            return SUCCESS;
        }
    });
    jobSetGateway.start();
    // We can't set default gateway if the address it points to is
    // not in the network of any link = bring eth0 up first
    ASSERT_TRUE(jobSetGateway.wait() == ERROR);
}

/*
 * Makes sure setting the gateway succeeds when we've brought the link up before
 */
TEST_F(NetlinkTest, SetGatewayWithUp) {
    // Bring the link up (fail if it is already up)
    FunctionJob jobLinkUp(*sc, [this] () {
        Netlink netlink;

        // Get eth0 status
        Netlink::LinkInfo linkDown;
        if (isError(netlink.findLink(IFACE, linkDown))) {
            return ERROR;
        }
        int linkIndex = linkDown.first.ifi_index;

        // Link was already up
        if ((linkDown.first.ifi_flags & IFF_UP) != 0) {
            return SUCCESS;
        }

        // Bring the link up
        if (isError(netlink.linkUp(linkIndex))) {
            return ERROR;
        }

        return SUCCESS;
    });
    jobLinkUp.start();
    ASSERT_TRUE(jobLinkUp.wait() == SUCCESS);

    FunctionJob jobSetIP(*sc, [this] () {
        Netlink netlink;
        Netlink::LinkInfo link;
        if (isError(netlink.findLink(IFACE, link))) {
            return ERROR;
        }
        int ifaceIndex = link.first.ifi_index;

        in_addr ip;
        if (inet_pton(AF_INET, IFACEADDR, &ip) != 1) {
            return ERROR;
        }

        if (isError(netlink.setIP(ifaceIndex, ip, NETMASK))) {
            return ERROR;
        }

        return SUCCESS;
    });
    jobSetIP.start();
    ASSERT_TRUE(jobSetIP.wait() == SUCCESS);

    // Set the gateway
    FunctionJob jobSetGateway(*sc, [this] () {
        Netlink netlink;
        if (isError(netlink.setDefaultGateway(GWADDR))) {
            return ERROR;
        }
        return SUCCESS;
    });
    jobSetGateway.start();
    ASSERT_TRUE(jobSetGateway.wait() == SUCCESS);
}

/*
 * Make sure that we can't find a link that does not exist
 */
TEST_F(NetlinkTest, FindUnexistingLink) {
    Netlink netlink;
    Netlink::LinkInfo link;
    memset(&link.first, 0, sizeof(link.first));

    ASSERT_FALSE(isSuccess(netlink.findLink("bajs3", link)));
    ASSERT_TRUE(link.second.empty());

    // Check that no fields were set
    unsigned int zero = 0;
    ASSERT_EQ(link.first.ifi_family, zero);
    ASSERT_EQ(link.first.ifi_type, zero);
    ASSERT_EQ(link.first.ifi_index, 0); // signed int
    ASSERT_EQ(link.first.ifi_flags, zero);
    ASSERT_EQ(link.first.ifi_change, zero);
}

/*
 * Creates a couple of test vectors for addresses and make sure that HasAddress succeeds and fails
 * where applicable.
 */
TEST_F(NetlinkTest, HasAddress) {
    Netlink netlink;

    // Dummy ifaddrmsg is not read by the function anyway, but is required for structure
    ifaddrmsg msg;

    // First test, just an empty message address list
    std::vector<Netlink::AddressInfo> empty;
    ASSERT_FALSE(isSuccess(netlink.hasAddress(empty, AF_INET, IFACEADDR)));
    ASSERT_FALSE(isSuccess(netlink.hasAddress(empty, AF_INET6, IFACEADDR)));

    // Second test, create some attributet without real data in them
    Netlink::AttributeList badList1;

    Netlink::AttributeInfo badAttr1;
    badAttr1.first.rta_type = IFA_ADDRESS;
    badAttr1.first.rta_len = 20;
    badAttr1.second = malloc(20);

    Netlink::AttributeInfo badAttr2;
    badAttr2.first.rta_type = IFA_ADDRESS;
    badAttr2.first.rta_len = 20;
    badAttr2.second = malloc(10);

    Netlink::AttributeInfo badAttr3;
    badAttr3.first.rta_type = IFA_ADDRESS;
    badAttr3.first.rta_len = 20;
    badAttr3.second = malloc(30);

    badList1.push_back(badAttr1);
    badList1.push_back(badAttr2);
    badList1.push_back(badAttr3);

    std::pair<ifaddrmsg, Netlink::AttributeList> badPair1;
    badPair1.first = msg;
    badPair1.second = badList1;

    std::vector<Netlink::AddressInfo> badData1;
    badData1.push_back(badPair1);

    ASSERT_FALSE(isSuccess(netlink.hasAddress(badData1, AF_INET, IFACEADDR)));
    ASSERT_FALSE(isSuccess(netlink.hasAddress(badData1, AF_INET6, IFACEADDR)));

    // Free since we did malloc
    free(badAttr1.second);
    free(badAttr2.second);
    free(badAttr3.second);

    // Third test, correct data but wrong IP number
    Netlink::AttributeList goodList1;

    Netlink::AttributeInfo goodAttr1;
    goodAttr1.first.rta_type = IFA_ADDRESS;

    in_addr wrongIP;
    inet_pton(AF_INET, "127.0.0.1", &wrongIP);

    goodAttr1.first.rta_len = sizeof(wrongIP);
    goodAttr1.second = &wrongIP;

    goodList1.push_back(goodAttr1);

    std::pair<ifaddrmsg, Netlink::AttributeList> goodPair1;
    goodPair1.first = msg;
    goodPair1.second = goodList1;

    std::vector<Netlink::AddressInfo> legitData1;
    legitData1.push_back(goodPair1);

    ASSERT_FALSE(isSuccess(netlink.hasAddress(legitData1, AF_INET, IFACEADDR)));
    ASSERT_FALSE(isSuccess(netlink.hasAddress(legitData1, AF_INET6, IFACEADDR)));

    // Fourth test, with correct IP
    Netlink::AttributeInfo goodAttr2;
    goodAttr2.first.rta_type = IFA_ADDRESS;
    in_addr correctIP;
    inet_pton(AF_INET, IFACEADDR, &correctIP);

    goodAttr2.first.rta_len = RTA_LENGTH(sizeof(correctIP));
    goodAttr2.second = &correctIP;

    Netlink::AttributeList goodList2;
    goodList2.push_back(goodAttr2);

    std::pair<ifaddrmsg, Netlink::AttributeList> goodPair2;
    goodPair2.first = msg;
    goodPair2.second = goodList2;

    std::vector<Netlink::AddressInfo> legitData2;
    legitData2.push_back(goodPair2);

    ASSERT_TRUE(isSuccess(netlink.hasAddress(legitData2, AF_INET, IFACEADDR)));
    ASSERT_FALSE(isSuccess(netlink.hasAddress(legitData2, AF_INET6, IFACEADDR)));

    // Fifth test, just try with some more stuff in the list
    std::vector<Netlink::AddressInfo> legitData3;
    legitData3.push_back(goodPair1);
    legitData3.push_back(goodPair2);

    ASSERT_TRUE(isSuccess(netlink.hasAddress(legitData3, AF_INET, IFACEADDR)));
    ASSERT_FALSE(isSuccess(netlink.hasAddress(legitData3, AF_INET6, IFACEADDR)));
}

/*
 * Test the FindAddresses function by getting all the addresses for the main
 * interface, and then making sure that hasAddress succeeds for each of those.
 */
TEST_F(NetlinkTest, FindAddresses) {
    FunctionJob jobFindAddresses(*sc, [this] () {
        Netlink netlink;

        Netlink::LinkInfo link;
        if (isError(netlink.findLink(IFACE, link))) {
            return ERROR;
        }

        std::vector<Netlink::AddressInfo> addresses;
        if (isError(netlink.findAddresses(link.first.ifi_index, addresses))) {
            return ERROR;
        }

        // For each of the addresses we got
        for (Netlink::AddressInfo addressInfo : addresses) {
            Netlink::AttributeList attributes = addressInfo.second;
            for (Netlink::AttributeInfo attrInfo : attributes) {
                rtattr attr = attrInfo.first;

                if (attr.rta_type == IFA_LOCAL || attr.rta_type == IFA_ADDRESS) {
                    // Convert it to a dotted notation ip address
                    char ip[INET6_ADDRSTRLEN];
                    if (inet_ntop(addressInfo.first.ifa_family, attrInfo.second, ip, sizeof(ip))) {
                        // And make sure that the hasAddress verifies it.
                        if (isError(netlink.hasAddress(addresses, addressInfo.first.ifa_family, ip))) {
                            return ERROR;
                        }
                    } else {
                        return ERROR;
                    }
                }
            }
        }

        // Just make sure some other addresses is not there
        std::vector<std::string> bogusIPs = { "0.0.0.0", "256.256.0.0", "127.0.0.1" };
        for (std::string ip : bogusIPs) {
            if (isSuccess(netlink.hasAddress(addresses, AF_INET, ip.c_str()))) {
                return ERROR;
            }

            if (isSuccess(netlink.hasAddress(addresses, AF_INET6, ip.c_str()))) {
                return ERROR;
            }
        }

        return SUCCESS;
    });
    jobFindAddresses.start();
    ASSERT_TRUE(jobFindAddresses.wait() == SUCCESS);
}
