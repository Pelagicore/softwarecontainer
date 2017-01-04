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

#include "gateway/network/iptableentry.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

using namespace softwarecontainer;

class MockIPTableEntry :
    public IPTableEntry
{
public:
    MockIPTableEntry():IPTableEntry()
    {
        m_type = "INPUT";
        m_defaultTarget = Target::DROP;
    }
};

class IPTableEntryTest : public ::testing::Test
{
protected:
    ::testing::NiceMock<MockIPTableEntry> ipTable;
};


/*
 * @brief Tests if DROP policy matched with default targets
 * */
TEST_F(IPTableEntryTest, Policy) {
    ipTable.m_defaultTarget = IPTableEntry::Target::DROP;
    ASSERT_EQ("iptables -P INPUT DROP", ipTable.interpretPolicy());
}

/*
 * @brief Tests if INPUT chain multiple port list rule is matched
 * */
TEST_F(IPTableEntryTest, InputMultiplePortList) {
    ipTable.m_type = "INPUT";
    IPTableEntry::Rule r;
    r.host = "127.0.0.1/16";
    r.ports = {true, true, "80,8080"};
    r.target = IPTableEntry::Target::ACCEPT;
    ASSERT_EQ("iptables -A INPUT -s 127.0.0.1/16 -p all --match multiport --sports 80,8080"
              " -j ACCEPT", ipTable.interpretRule(r));
}

/*
 * @brief Tests if INPUT chain multiple port range with protocol udp rule is matched
 * */
TEST_F(IPTableEntryTest, InputMultiplePortRange) {
    ipTable.m_type = "INPUT";
    IPTableEntry::Rule r;
    r.host = "127.0.0.1/16";
    r.ports = {true, true, "80:85"};
    r.target = IPTableEntry::Target::ACCEPT;
    r.protocols = {"udp"};
    ASSERT_EQ("iptables -A INPUT -s 127.0.0.1/16 -p udp --match multiport --sports 80:85"
              " -j ACCEPT", ipTable.interpretRuleWithProtocol(r, r.protocols[0]));
}

/*
 * @brief Tests if INPUT chain single port with icmp protocol rule is matched
 * */
TEST_F(IPTableEntryTest, InputSinglePort) {
    ipTable.m_type = "INPUT";
    IPTableEntry::Rule r;
    r.host = "127.0.0.1/16";
    r.ports = {true, false, "80"};
    r.target = IPTableEntry::Target::ACCEPT;
    r.protocols = {"icmp"};
    ASSERT_EQ("iptables -A INPUT -s 127.0.0.1/16 -p icmp --sport 80"
              " -j ACCEPT", ipTable.interpretRuleWithProtocol(r, r.protocols[0]));

}

/*
 * @brief Tests if OUTPUT chain multiple port list with udp protocol rule is matched
 * */
TEST_F(IPTableEntryTest, OutputMultiplePortList) {
    ipTable.m_type = "OUTPUT";
    IPTableEntry::Rule r;
    r.host = "127.0.0.1/16";
    r.ports = {true, true, "80,8080"};
    r.target = IPTableEntry::Target::ACCEPT;
    r.protocols= {"udp"};
    ASSERT_EQ("iptables -A OUTPUT -d 127.0.0.1/16 -p udp --match multiport --dports 80,8080"
              " -j ACCEPT", ipTable.interpretRuleWithProtocol(r, r.protocols[0]));
}

/*
 * @brief Tests if OUTPUT chain multiple port list rule is matched
 * */
TEST_F(IPTableEntryTest, OutputMultiplePortRange) {
    ipTable.m_type = "OUTPUT";
    IPTableEntry::Rule r;
    r.host = "127.0.0.1/16";
    r.ports = {true, true, "80:85"};
    r.protocols= {"tcp"};
    r.target = IPTableEntry::Target::ACCEPT;
    ASSERT_EQ("iptables -A OUTPUT -d 127.0.0.1/16 -p tcp --match multiport --dports 80:85"
              " -j ACCEPT", ipTable.interpretRuleWithProtocol(r, r.protocols[0]));
}

/*
 * @brief Tests if OUTPUT chain multiple port list rule is matched
 * */
TEST_F(IPTableEntryTest, IPTableEntryOutputSinglePort) {
    ipTable.m_type = "OUTPUT";
    IPTableEntry::Rule r;
    r.host = "127.0.0.1/16";
    r.ports = {true, false, "80"};
    r.protocols= {"tcp"};
    r.target = IPTableEntry::Target::ACCEPT;
    ASSERT_EQ("iptables -A OUTPUT -d 127.0.0.1/16 -p tcp --dport 80"
              " -j ACCEPT", ipTable.interpretRule(r));
}

