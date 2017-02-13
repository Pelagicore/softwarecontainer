/*
 * Copyright (C) 2016-2017 Pelagicore AB
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

#include "gateway/dbus/dbusgatewayinstance.h"
#include "softwarecontainer-common.h"

class MockDBusGatewayInstance : public DBusGatewayInstance
{
public:
    MockDBusGatewayInstance(ProxyType type,
                            const std::string &gatewayDir,
                            std::shared_ptr<ContainerAbstractInterface> container) :
        DBusGatewayInstance(type, gatewayDir, container)
    {
    }

    MOCK_METHOD2(startDBusProxy, bool(const std::vector<std::string> &commandVec, const std::vector<std::string> &envVec));
    MOCK_METHOD1(testDBusConnection, bool(const std::string &config));

};

using ::testing::_;

class DBusGatewayInstanceTest :
    public SoftwareContainerGatewayTest,
    public ::testing::WithParamInterface<std::pair<DBusGatewayInstance::ProxyType, std::string>>
{
public:
    const std::string m_gatewayDir = "/tmp/dbusgateway-unit-test/";
    DBusGatewayInstance::ProxyType m_proxyType;
    std::string m_configType;

    void SetUp() override
    {
        SoftwareContainerGatewayTest::SetUp();

        auto pair = GetParam();
        m_proxyType = pair.first;
        m_configType = pair.second;
    }
};

/*
 * Test that activate works with a proper full session config
 */
TEST_P(DBusGatewayInstanceTest, ActivateBus) {
    const std::string config = "[{"
            "\"" + m_configType + "\": [{"
               "\"direction\": \"*\","
               "\"interface\": \"*\","
               "\"object-path\": \"*\","
               "\"method\": \"*\""
            "}]}]";
    loadConfig(config);

    MockDBusGatewayInstance gw(m_proxyType, m_gatewayDir, m_container);
    ASSERT_TRUE(gw.setConfig(jsonConfig));

    ::testing::DefaultValue<bool>::Set(true);
    {
        ::testing::InSequence sequence;
        EXPECT_CALL(gw, startDBusProxy(_, _));
        EXPECT_CALL(gw, testDBusConnection(_));
    }

    ASSERT_TRUE(gw.activate());
}

INSTANTIATE_TEST_CASE_P(ActivationWorks,
                        DBusGatewayInstanceTest,
                        ::testing::Values(
                            std::make_pair(DBusGatewayInstance::ProxyType::SystemProxy,
                                           DBusGatewayInstance::SYSTEM_CONFIG),
                            std::make_pair(DBusGatewayInstance::ProxyType::SessionProxy,
                                           DBusGatewayInstance::SESSION_CONFIG)
                        ));
