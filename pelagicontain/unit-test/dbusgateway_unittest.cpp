/*
 *   Copyright (C) 2014 Pelagicore AB
 *   All rights reserved.
 */
#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include <iostream>

#include "dbusgateway.h"
#include "pelagicontain-common.h"

using namespace pelagicore;

class MockController
{
public:
    virtual bool startApp()
    {
        return true;
    }

    virtual bool shutdown()
    {
        return true;
    }

    virtual ReturnCode setEnvironmentVariable(const std::string &variable,
            const std::string &value)
    {
        return ReturnCode::SUCCESS;
    }

    virtual bool hasBeenStarted() const
    {
        return true;
    }

    virtual bool initialize()
    {
        return true;
    }

    MOCK_METHOD1( systemCall, ReturnCode(const std::string & cmd) );

};

void close_fd_helper(pid_t pid, int infp, int outfp)
{
    close(infp);
}


using::testing::InSequence;
using::testing::_;
using::testing::Return;
using::testing::NiceMock;

class DBusGatewayTest :
    public::testing::Test
{
public:
    const std::string m_gatewayDir = "/tmp/dbusgateway-unit-test/gateways";
    const std::string m_containerName = "test";
    NiceMock<MockController> controllerInterface;
};

/*! Test DBusGateway saves the config when DBusGateway::setConfig()
 * has been called.
 */
TEST_F(DBusGatewayTest, TestSetConfig) {
    DBusGateway gw(
            DBusGateway::SessionProxy,
            m_gatewayDir,
            m_containerName);

    std::string config = "{}";
    bool success = gw.setConfig(config);
    ASSERT_TRUE(success);
}

/*! Test DBusGateway writes the config provided by DBusGateway::setConfig()
 * to a fileno provided by the systemcallInterface when
 * DBusGateway::activate() has been called.
 */
TEST_F(DBusGatewayTest, TestActivateStdInWrite) {
    DBusGateway gw(
            DBusGateway::SessionProxy,
            m_gatewayDir,
            m_containerName);

    std::string config = "{}";

    ASSERT_TRUE( gw.setConfig(config) );

    // DBusGateway relies on dbus-proxy to create a socket and since dbus-proxy
    // will not be started during these tests we need to create a file so
    // DBusGateway doesn't fail
    std::string socketFile = m_gatewayDir + "/sess_" + m_containerName + ".sock";
    std::string cmd_mkdir = "mkdir -p ";
    cmd_mkdir += m_gatewayDir;
    system( cmd_mkdir.c_str() );
    std::string cmd_touch = "touch ";
    cmd_touch += socketFile;
    system( cmd_touch.c_str() );

    ASSERT_TRUE( gw.activate() );

    // Teardown will remove the "socket" file created above.
    gw.teardown();
}

/*! Test DBusGateway calls ControllerInterface::makePopencall() when
 * DBusGateway::activate() has been called
 *
 * The DbusGateway::activate() should try to issue a dbus-proxy call
 * and then try to write the config to stdin of this. At the end it
 * should remove the file created by dbus-proxy.
 */
TEST_F(DBusGatewayTest, TestActivateCall) {
    DBusGateway *gw = new DBusGateway(
            DBusGateway::SessionProxy,
            m_gatewayDir,
            m_containerName);

    // create sock file which teardown will remove
    std::string cmd_mkdir = "mkdir -p ";
    cmd_mkdir += m_gatewayDir;
    system( cmd_mkdir.c_str() );
    std::string cmd_touch = "touch ";
    cmd_touch += m_gatewayDir;
    cmd_touch += "/sess_test.sock";
    system( cmd_touch.c_str() );

    std::string config = "{}";

    ASSERT_TRUE( gw->setConfig(config) );

    FILE *tmp = tmpfile();
    int infp = fileno(tmp);

    {
        InSequence sequence;
        EXPECT_CALL(
                systemcallInterfaceMock,
                makePopenCall("dbus-proxy "
                    "/tmp/dbusgateway-unit-test/gateways/sess_test.sock "
                    "session",
                    _, _)
                ).WillOnce( DoAll( ::testing::SetArgPointee<1>(infp), Return(999) ) );

        EXPECT_CALL(
                systemcallInterfaceMock,
                makePcloseCall(_, _, _)
                ).WillOnce( DoAll( ::testing::Invoke(close_fd_helper), Return(true) ) );
    }

    ASSERT_TRUE( gw->activate() );
    ASSERT_TRUE( gw->teardown() );

    delete gw;
}
