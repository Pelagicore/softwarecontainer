
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

#include "softwarecontainer-common.h"
#include "softwarecontainer_test.h"
#include "gateway/filegateway.h"
#include "functionjob.h"
#include <unistd.h>
#include <stdlib.h>

class FileGatewayTest : public SoftwareContainerGatewayTest
{

public:
    FileGatewayTest() { }
    FileGateway *gw;

    void SetUp() override
    {
        gw = new FileGateway();
        SoftwareContainerTest::SetUp();

        // Create file
        ASSERT_TRUE(writeToFile(FILE_PATH, FILE_CONTENT) == ReturnCode::SUCCESS);
    }

    void TearDown() override
    {
        SoftwareContainerTest::TearDown();

        // Remove file
        unlink(FILE_PATH.c_str());
    }

    const std::string FILE_CONTENT = "ahdkhqweuyreqiwenomndlaskmd";
    const std::string FILE_PATH = "/tmp/filename.txt";
    const std::string CONTAINER_PATH = "/filename.txt";
    const std::string ENV_VAR_NAME = "TEST_ENVIRONMENT_VARIABLE_NAME";
    const std::string PREFIX = "TEST_PREFIX";
    const std::string SUFFIX = "TEST_SUFFIX";
};

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

    ASSERT_TRUE(existsInFileSystem(FILE_PATH));
}

TEST_F(FileGatewayTest, TestActivateSetEnvWPrefixAndSuffix) {
    givenContainerIsSet(gw);
    FunctionJob job = FunctionJob(*sc, [&] () {
        const char* envC = getenv(ENV_VAR_NAME.c_str());

        if (envC == nullptr) {
            return 1;
        }

        const std::string env = std::string(envC);
        if (env.length() < PREFIX.length() + SUFFIX.length()) {
            return 2;
        }

        for (size_t i = 0; i < PREFIX.length(); ++i) {
            if (env[i] != PREFIX[i]) {
               return 3;
            }
        }

        const int suffixStart = env.length() - SUFFIX.length();
        for (size_t i = 0; i < SUFFIX.length(); ++i) {
            if (env[suffixStart + i] != SUFFIX[i]) {
               return 4;
            }
        }

        return 0;
    });

    job.start();
    ASSERT_EQ(job.wait(), 1);

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

    job.start();
    ASSERT_EQ(job.wait(), 0);
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
