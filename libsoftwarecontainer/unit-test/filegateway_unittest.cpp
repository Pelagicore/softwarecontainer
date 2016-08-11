/*
 *   Copyright (C) 2016 Pelagicore AB
 *   All rights reserved.
 */

#include "softwarecontainer_test.h"
#include "gateway/filegateway.h"

class FileGatewayTest : public SoftwareContainerGatewayTest
{

public:
    FileGatewayTest() { }
    FileGateway *gw;

    void SetUp() override
    {
        gw = new FileGateway();
        SoftwareContainerLibTest::SetUp();

        // Create file
        std::string cmd = "echo " + FILE_CONTENT + " > " + FILE_PATH;
        ASSERT_TRUE(system(cmd.c_str()) == 0);
    }

    void TearDown() override
    {
        SoftwareContainerLibTest::TearDown();

        // Remove file
        std::string cmd = "rm " + FILE_PATH;
        system(cmd.c_str());
    }

    bool runInShell(std::string cmd) {
        CommandJob job(*lib, "sh -c \"" + cmd + "\"");
        job.start();
        return job.wait() == 0;
    }

    const std::string FILE_CONTENT = "ahdkhqweuyreqiwenomndlaskmd";
    const std::string FILE_PATH = "/tmp/filename.txt";
    const std::string CONTAINER_PATH = "/filename.txt";
    const std::string ENV_VAR_NAME = "TEST_ENVIRONMENT_VARIABLE_NAME";
    const std::string PREFIX = "TEST_PREFIX";
    const std::string SUFFIX = "TEST_SUFFIX";
};

TEST_F(FileGatewayTest, TestActivateWithNoConf) {
    givenContainerIsSet(gw);
    ASSERT_FALSE(gw->activate());
}

TEST_F(FileGatewayTest, TestActivateWithEmptyValidJSONConf) {
    givenContainerIsSet(gw);
    const std::string config = "[]";

    ASSERT_TRUE(gw->setConfig(config));
    ASSERT_FALSE(gw->activate());
}


TEST_F(FileGatewayTest, TestActivateWithMinimalValidConf) {
    givenContainerIsSet(gw);
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
    givenContainerIsSet(gw);
    const std::string cmd = "cat " + FILE_PATH;
    ASSERT_FALSE(runInShell(cmd));
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

    ASSERT_TRUE(runInShell(cmd));
}

TEST_F(FileGatewayTest, TestActivateSetEnvWPrefixAndSuffix) {
    givenContainerIsSet(gw);
    const std::string cmd =  "printenv"
                             "| grep " + ENV_VAR_NAME +
                             "| grep " + PREFIX +
                             "| grep " + SUFFIX;
    ASSERT_FALSE(runInShell(cmd));

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

    ASSERT_TRUE(runInShell(cmd));
}

TEST_F(FileGatewayTest, TestActivateWithNoPathToHost) {
    givenContainerIsSet(gw);
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
    givenContainerIsSet(gw);
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
    givenContainerIsSet(gw);
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
    givenContainerIsSet(gw);
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
    delete gw;
}
