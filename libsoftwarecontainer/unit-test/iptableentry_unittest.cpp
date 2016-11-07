
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


#include "gateway/iptableentry.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

class MockIPTableEntry :
    public IPTableEntry
{
public:
    MockIPTableEntry():IPTableEntry()
    {
        m_type = "INPUT";
        m_defaultTarget = Target::ACCEPT;
    }
};

class IPTableEntryTest : public ::testing::Test
{
protected:
    ::testing::NiceMock<MockIPTableEntry> ipTable;
};

/*
 * @brief Tests if ACCEPT policy matched with default targets
 * */
TEST_F(IPTableEntryTest, TestIPTableEntryAcceptPolicy) {
    ipTable.m_defaultTarget = IPTableEntry::Target::ACCEPT;
    ASSERT_EQ("iptables -P INPUT ACCEPT", ipTable.interpretPolicy());
}

/*
 * @brief Tests if DROP policy matched with default targets
 * */
TEST_F(IPTableEntryTest, TestIPTableEntryDropPolicy) {
    ipTable.m_defaultTarget = IPTableEntry::Target::DROP;
    ASSERT_EQ("iptables -P INPUT DROP", ipTable.interpretPolicy());
}

/*
 * @brief Tests IPTableEntry behavior when run accross wrong default target
 * */
TEST_F(IPTableEntryTest, TestIPTableEntryWrongPolicy) {
    ipTable.m_defaultTarget = IPTableEntry::Target::REJECT;
    ASSERT_EQ("", ipTable.interpretPolicy());
}

/*
 * @brief Tests if INPUT chain multiple port list rule is matched
 * */
TEST_F(IPTableEntryTest, TestIPTableEntryInputMultiplePortList) {
    ipTable.m_type = "INPUT";
    IPTableEntry::Rule r;
    r.host = "127.0.0.1/16";
    r.ports = {true, true, "80,8080"};
    r.target = IPTableEntry::Target::ACCEPT;
    ASSERT_EQ("iptables -A INPUT -s 127.0.0.1/16 -p tcp --match multiport --sports 80,8080"
                " -j ACCEPT", ipTable.interpretRule(r));
}

/*
 * @brief Tests if INPUT chain multiple port range rule is matched
 * */
TEST_F(IPTableEntryTest, TestIPTableEntryInputMultiplePortRange) {
    ipTable.m_type = "INPUT";
    IPTableEntry::Rule r;
    r.host = "127.0.0.1/16";
    r.ports = {true, true, "80:85"};
    r.target = IPTableEntry::Target::ACCEPT;
    ASSERT_EQ("iptables -A INPUT -s 127.0.0.1/16 -p tcp --match multiport --sports 80:85"
            " -j ACCEPT", ipTable.interpretRule(r));
}

/*
 * @brief Tests if INPUT chain single port rule is matched
 * */
TEST_F(IPTableEntryTest, TestIPTableEntryInputSinglePort) {
    ipTable.m_type = "INPUT";
    IPTableEntry::Rule r;
    r.host = "127.0.0.1/16";
    r.ports = {true, false, "80"};
    r.target = IPTableEntry::Target::ACCEPT;
    ASSERT_EQ("iptables -A INPUT -s 127.0.0.1/16 -p tcp --sport 80"
            " -j ACCEPT", ipTable.interpretRule(r));

}

/*
 * @brief Tests if OUTPUT chain multiple port list rule is matched
 * */
TEST_F(IPTableEntryTest, TestIPTableEntryOutputMultiplePortList) {
    ipTable.m_type = "OUTPUT";
    IPTableEntry::Rule r;
    r.host = "127.0.0.1/16";
    r.ports = {true, true, "80,8080"};
    r.target = IPTableEntry::Target::ACCEPT;
    ASSERT_EQ("iptables -A OUTPUT -d 127.0.0.1/16 -p tcp --match multiport --dports 80,8080"
            " -j ACCEPT", ipTable.interpretRule(r));
}

/*
 * @brief Tests if OUTPUT chain multiple port list rule is matched
 * */
TEST_F(IPTableEntryTest, TestIPTableEntryOutputMultiplePortRange) {
    ipTable.m_type = "OUTPUT";
    IPTableEntry::Rule r;
    r.host = "127.0.0.1/16";
    r.ports = {true, true, "80:85"};
    r.target = IPTableEntry::Target::ACCEPT;
    ASSERT_EQ("iptables -A OUTPUT -d 127.0.0.1/16 -p tcp --match multiport --dports 80:85"
            " -j ACCEPT", ipTable.interpretRule(r));
}

/*
 * @brief Tests if OUTPUT chain multiple port list rule is matched
 * */
TEST_F(IPTableEntryTest, TestIPTableEntryOutputSinglePort) {
    ipTable.m_type = "OUTPUT";
    IPTableEntry::Rule r;
    r.host = "127.0.0.1/16";
    r.ports = {true, false, "80"};
    r.target = IPTableEntry::Target::ACCEPT;
    ASSERT_EQ("iptables -A OUTPUT -d 127.0.0.1/16 -p tcp --dport 80"
            " -j ACCEPT", ipTable.interpretRule(r));
}
