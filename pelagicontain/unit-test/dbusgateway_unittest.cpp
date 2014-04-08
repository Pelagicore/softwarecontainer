/*
 *   Copyright (C) 2014 Pelagicore AB
 *   All rights reserved.
 */
#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include <iostream>

#include "dbusgateway.h"
#include "debug.h"
#include "log_console.h"

using namespace pelagicore;

class MockController :
    public ControllerAbstractInterface
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

    virtual bool setEnvironmentVariable(const std::string &variable,
        const std::string &value)
    {
        return true;
    }

    MOCK_METHOD1(systemCall, bool(const std::string &cmd));

};

class MockSystemcallInnterface :
    public SystemcallAbstractInterface
{
public:
    MOCK_METHOD1(makeCall, bool(const std::string &cmd));
    MOCK_METHOD2(makeCall, bool(const std::string &cmd, int &exitCode));
    MOCK_METHOD3(makePopenCall,
                 pid_t(const std::string &command, int *infp, int *outfp));
    MOCK_METHOD3(makePcloseCall, bool(pid_t pid, int infp, int outfp));
};

void close_fd_helper(pid_t pid, int infp, int outfp)
{
    close(infp);
}


class SystemcallInterfaceStub :
    public SystemcallAbstractInterface
{
public:
    FILE *file_descriptor = NULL;
    pid_t m_pid = 999;
    int m_infp = -1;
    int m_outfp = -1;

    SystemcallInterfaceStub() {
        file_descriptor = tmpfile();
        m_infp = fileno(file_descriptor);
    }

    virtual ~SystemcallInterfaceStub() {
        if(file_descriptor != NULL) {
            fclose(file_descriptor);
        }
    };

    virtual bool makeCall(const std::string &cmd)
    {
        return true;
    }

    virtual bool makeCall(const std::string &cmd, int &exitCode)
    {
        exitCode = 0;
        return true;
    }

    pid_t makePopenCall(const std::string &command,
                        int *infp,
                        int *outfp)
    {
        *infp = m_infp;
        *outfp = m_outfp;
        return m_pid;
    }

    bool makePcloseCall(pid_t pid, int infp, int outfp)
    {
        if(pid == m_pid && infp == m_infp && outfp == m_outfp) {
            return true;
        }

        return false;
    }

    std::string fileContent()
    {
        std::string content = "";
        char buf[20];
        rewind(file_descriptor);
        while (fgets(buf, 20, file_descriptor)) {
            content += buf;
        }

        return content;
    }
};


using ::testing::InSequence;
using ::testing::_;
using ::testing::Return;
using ::testing::NiceMock;

class DBusGatewayTest : public ::testing::Test
{
public:
    const std::string m_gatewayDir = "/tmp/dbusgateway-unit-test/gateways";
    const std::string m_containerName = "test";
    NiceMock<MockController> controllerInterface;
    SystemcallInterfaceStub systemcallInterface;
};

/*! Test DBusGateway saves the config when DBusGateway::setConfig()
 * has been called.
 */
TEST_F(DBusGatewayTest, TestSetConfig) {
    DBusGateway gw(&controllerInterface,
                   &systemcallInterface,
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
    DBusGateway gw(&controllerInterface,
                   &systemcallInterface,
                   DBusGateway::SessionProxy,
                   m_gatewayDir,
                   m_containerName);

    std::string config = "{}";

    ASSERT_TRUE(gw.setConfig(config));
    ASSERT_TRUE(gw.activate());
    EXPECT_EQ(config, systemcallInterface.fileContent());
}

/*! Test DBusGateway calls ControllerInterface::makePopencall() when
 * DBusGateway::activate() has been called
 *
 * The DbusGateway::activate() should try to issue a dbus-proxy call
 * and then try to write the config to stdin of this. At the end it
 * should remove the file created by dbus-proxy.
 */
TEST_F(DBusGatewayTest, TestActivateCall) {
    NiceMock<MockSystemcallInnterface> systemcallInterfaceMock;
    DBusGateway *gw = new DBusGateway(&controllerInterface,
                                      &systemcallInterfaceMock,
                                      DBusGateway::SessionProxy,
                                      m_gatewayDir,
                                      m_containerName);

    // create sock file which teardown will remove
    std::string cmd_mkdir = "mkdir -p ";
    cmd_mkdir += m_gatewayDir;
    system(cmd_mkdir.c_str());
    std::string cmd_touch = "touch ";
    cmd_touch += m_gatewayDir;
    cmd_touch += "/sess_test.sock";
    system(cmd_touch.c_str());

    std::string config = "{}";

    ASSERT_TRUE(gw->setConfig(config));

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
        ).WillOnce(DoAll(::testing::SetArgPointee<1>(infp), Return(999)));

        EXPECT_CALL(
            systemcallInterfaceMock,
            makePcloseCall(_, _, _)
        ).WillOnce(DoAll(::testing::Invoke(close_fd_helper), Return(true)));
    }

    ASSERT_TRUE(gw->activate());
    ASSERT_TRUE(gw->teardown());

    delete gw;
}

