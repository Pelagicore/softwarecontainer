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

    MOCK_METHOD1(systemCall,
        bool(const std::string &cmd));

};

class MockSystemcallInnterface :
    public SystemcallAbstractInterface
{
public:
    MOCK_METHOD1(makeCall, bool(const std::string &cmd));
    MOCK_METHOD2(makeCall, bool(const std::string &cmd, int &exitCode));
    MOCK_METHOD3(makePopenCall, bool(const std::string &command, const std::string &type, FILE **fd));
    MOCK_METHOD2(makePcloseCall, bool(FILE **fd, int &exitCode));
};

void close_fd_helper(FILE **fd, int &exitCode)
{
    exitCode = fclose(*fd) == 0;
}


class SystemcallInterfaceStub :
    public SystemcallAbstractInterface
{
public:
    FILE *file_descriptor = NULL;

    SystemcallInterfaceStub() {
        file_descriptor = tmpfile();
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

    bool makePopenCall(const std::string &command, const std::string &type, FILE **fd)
    {
        *fd = file_descriptor;
        return true;
    }

    bool makePcloseCall(FILE **fd, int &exitCode)
    {
        if(*fd == file_descriptor) {
            exitCode = 0;
            return true;
        }

        exitCode = -1;
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
    const std::string m_containerRoot = "/tmp/dbusgateway-unit-test";
    const std::string m_containerName = "test";
    NiceMock<MockController> controllerInterface;
  SystemcallInterfaceStub systemcallInterface;
};

/*! Test DBusGateway calls ControllerInterface::makePopencall when
 * DBusGateway::setConfig() has been called
 *
 * The DbusGateway::setConfig() should try to issue a ifconfig
 * system call inside the container in order to set the containers IP
 * address.
 */

TEST_F(DBusGatewayTest, TestSetConfig) {
    DBusGateway gw(&controllerInterface, &systemcallInterface, DBusGateway::SessionProxy, m_containerRoot, m_containerName);

    std::string config = "{}";
    bool success = gw.setConfig(config);
    ASSERT_TRUE(success);
}

TEST_F(DBusGatewayTest, TestActivateStdInWrite) {
    DBusGateway gw(&controllerInterface, &systemcallInterface, DBusGateway::SessionProxy, m_containerRoot, m_containerName);

    std::string config = "{}";

    ASSERT_TRUE(gw.setConfig(config));
    ASSERT_TRUE(gw.activate());
    EXPECT_EQ(config, systemcallInterface.fileContent());
}

TEST_F(DBusGatewayTest, TestActivateCall) {
    NiceMock<MockSystemcallInnterface> systemcallInterfaceMock;
    DBusGateway *gw = new DBusGateway(&controllerInterface, &systemcallInterfaceMock, DBusGateway::SessionProxy, m_containerRoot, m_containerName);

    std::string config = "{}";

    ASSERT_TRUE(gw->setConfig(config));

    FILE *tmp = tmpfile();

    {
        InSequence sequence;
        EXPECT_CALL(
            systemcallInterfaceMock,
            makePopenCall("dbus-proxy /tmp/dbusgateway-unit-test/gateways/sess_test.sock session", "w", _)
        ).WillOnce(DoAll(::testing::SetArgPointee<2>(tmp), Return(true)));

        EXPECT_CALL(
            systemcallInterfaceMock,
            makePcloseCall(_, _)
        ).WillOnce(DoAll(::testing::Invoke(close_fd_helper), ::testing::SetArgReferee<1>(0), Return(true)));
    }

    ASSERT_TRUE(gw->activate());

    delete gw;

}
