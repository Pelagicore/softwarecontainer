/*
 *   Copyright (C) 2016 Pelagicore AB
 *   All rights reserved.
 */

#include <thread>
#include <sys/un.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "generators.h"
#include "libpelagicontain.h"

#include "gateway/filegateway.h"

class FileGatewayTest :
    public::testing::Test
{

public:
    FileGatewayTest() { }

    void SetUp() override
    {
        ::testing::Test::SetUp();
        gw = std::unique_ptr<FileGateway>(new FileGateway());

        workspace = std::unique_ptr<PelagicontainWorkspace>(new PelagicontainWorkspace());
        lib = std::unique_ptr<PelagicontainLib>(new PelagicontainLib(*workspace));
        lib->setContainerIDPrefix("Test-");
        lib->setMainLoopContext(m_context);
        ASSERT_TRUE(isSuccess(lib->init()));
        std::string cmd = "echo " + FILE_CONTENT + " > " + FILE_PATH;
        ASSERT_TRUE(system(cmd.c_str()) == 0);
    }

    void TearDown() override
    {
        ::testing::Test::TearDown();

        // Ensuring that the reset is done in correct order
        lib.reset();
        gw.reset();
        workspace.reset();
        std::string cmd = "rm " + FILE_PATH;
        system(cmd.c_str());
    }

    void givenContainerIsSet() {
        lib->getPelagicontain().addGateway(*gw);
    }

    Glib::RefPtr<Glib::MainContext> m_context = Glib::MainContext::get_default();
    std::unique_ptr<FileGateway> gw;
    std::unique_ptr<PelagicontainLib> lib;
    std::unique_ptr<PelagicontainWorkspace> workspace;

    const std::string FILE_CONTENT = "ahdkhqweuyreqiwenomndlaskmd";
    const std::string FILE_PATH = "/tmp/filename.txt";
    const std::string CONTAINER_PATH = "/filename.txt";
    const std::string ENV_VAR_NAME = "TEST_ENVIRONMENT_VARIABLE_NAME";
    const std::string PREFIX = "TEST_PREFIX";
    const std::string SUFFIX = "TEST_SUFFIX";
};

TEST_F(FileGatewayTest, TestActivateWithNoConf) {
    givenContainerIsSet();
    ASSERT_FALSE(gw->activate());
}

TEST_F(FileGatewayTest, TestActivateWithEmptyValidJSONConf) {
    givenContainerIsSet();
    const std::string config = "[]";

    ASSERT_TRUE(gw->setConfig(config));
    ASSERT_FALSE(gw->activate());
}


TEST_F(FileGatewayTest, TestActivateWithMinimalValidConf) {
    givenContainerIsSet();
    const std::string config = 
    "["
        "{"
            "  \"path-host\" : \"" + FILE_PATH + "\""
            ", \"path-container\" : \"" + CONTAINER_PATH + "\""
        "}"
    "]";
    ASSERT_TRUE(gw->setConfig(config));
    ASSERT_TRUE(gw->activate());
}

TEST_F(FileGatewayTest, TestActivateCreateSymlink) {
    givenContainerIsSet();
    CommandJob job(*lib, "cat " + FILE_PATH);
    ASSERT_EQ(job.start(), ReturnCode::SUCCESS);
    ASSERT_TRUE(job.wait() != 0);

    const std::string config = 
    "["
        "{"
            "  \"path-host\" : \"" + FILE_PATH + "\""
            ", \"path-container\" : \"" + CONTAINER_PATH + "\""
            ", \"create-symlink\" : true"
        "}"
    "]";
    ASSERT_TRUE(gw->setConfig(config));
    ASSERT_TRUE(gw->activate());

    ASSERT_EQ(job.start(), ReturnCode::SUCCESS);
    ASSERT_TRUE(job.wait() == 0);
}

TEST_F(FileGatewayTest, TestActivateSetEnvWPrefixAndSuffix) {
    givenContainerIsSet();
    CommandJob job(*lib, "printenv | grep " + ENV_VAR_NAME + " | grep " + PREFIX + " | grep " + SUFFIX);
    ASSERT_EQ(job.start(), ReturnCode::SUCCESS);
    ASSERT_TRUE(job.wait() != 0);

    const std::string config = 
    "["
        "{"
            "  \"path-host\" : \"" + FILE_PATH + "\""
            ", \"path-container\" : \"" + CONTAINER_PATH + "\""
            ", \"env-var-name\": \"" + ENV_VAR_NAME + "\""
            ", \"env-var-prefix\": \"" + PREFIX + "\""
            ", \"env-var-suffix\": \"" + SUFFIX + "\""
        "}"
    "]";
    ASSERT_TRUE(gw->setConfig(config));
    ASSERT_TRUE(gw->activate());

    ASSERT_EQ(job.start(), ReturnCode::SUCCESS);
    ASSERT_TRUE(job.wait() == 0);
}

TEST_F(FileGatewayTest, TestActivateWithNoPathToHost) {
    givenContainerIsSet();
    const std::string config = 
    "["
        "{"
            "  \"path-container\" : \"" + CONTAINER_PATH + "\""
            ", \"create-symlink\" : false"
            ", \"read-only\": true"
            ", \"env-var-name\": \"" + ENV_VAR_NAME + "\""
            ", \"env-var-prefix\": \"" + PREFIX + "\""
            ", \"env-var-suffix\": \"" + SUFFIX + "\""
        "}"
    "]";
    ASSERT_FALSE(gw->setConfig(config));
    ASSERT_FALSE(gw->activate());
}

TEST_F(FileGatewayTest, TestActivateWithEmptyPathToHost) {
    givenContainerIsSet();
    const std::string config = 
    "["
        "{"
            "  \"path-host\" : \"\""
            ", \"path-container\" : \"" + CONTAINER_PATH + "\""
            ", \"create-symlink\" : false"
            ", \"read-only\": true"
            ", \"env-var-name\": \"" + ENV_VAR_NAME + "\""
            ", \"env-var-prefix\": \"" + PREFIX + "\""
            ", \"env-var-suffix\": \"" + SUFFIX + "\""
        "}"
    "]";
    ASSERT_FALSE(gw->setConfig(config));
    ASSERT_FALSE(gw->activate());
}
TEST_F(FileGatewayTest, TestActivateWithNoPathInContainer) {
    givenContainerIsSet();
    const std::string config = 
    "["
        "{"
            "  \"path-home\" : \"" + FILE_PATH + "\""
            ", \"create-symlink\" : false"
            ", \"read-only\": true"
            ", \"env-var-name\": \"" + ENV_VAR_NAME + "\""
            ", \"env-var-prefix\": \"" + PREFIX + "\""
            ", \"env-var-suffix\": \"" + SUFFIX + "\""
        "}"
    "]";
    ASSERT_FALSE(gw->setConfig(config));
    ASSERT_FALSE(gw->activate());
}

TEST_F(FileGatewayTest, TestActivateWithEmptyPathInContainer) {
    givenContainerIsSet();
    const std::string config = 
    "["
        "{"
            "  \"path-home\" : \"" + FILE_PATH + "\""
            ", \"path-container\" : \"\""
            ", \"create-symlink\" : false"
            ", \"read-only\": true"
            ", \"env-var-name\": \"" + ENV_VAR_NAME + "\""
            ", \"env-var-prefix\": \"" + PREFIX + "\""
            ", \"env-var-suffix\": \"" + SUFFIX + "\""
        "}"
    "]";
    ASSERT_FALSE(gw->setConfig(config));
    ASSERT_FALSE(gw->activate());
}

TEST_F(FileGatewayTest, TestActivateWithNoContainer) {
    const std::string config = 
    "["
        "{"
            "  \"path-host\" : \"" + FILE_PATH + "\""
            ", \"path-container\" : \"" + CONTAINER_PATH + "\""
            ", \"create-symlink\" : false"
            ", \"read-only\": true"
            ", \"env-var-name\": \"" + ENV_VAR_NAME + "\""
            ", \"env-var-prefix\": \"" + PREFIX + "\""
            ", \"env-var-suffix\": \"" + SUFFIX + "\""
        "}"
    "]";
    ASSERT_TRUE(gw->setConfig(config));
    ASSERT_FALSE(gw->activate());
}
