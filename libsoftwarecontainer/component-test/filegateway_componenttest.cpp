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

#include "softwarecontainer-common.h"
#include "softwarecontainer_test.h"
#include "gateway/files/filegateway.h"
#include "functionjob.h"
#include <unistd.h>
#include <stdlib.h>
#include <fstream>

class FileGatewayTest : public SoftwareContainerGatewayTest
{

public:
    FileGatewayTest() { }
    std::unique_ptr<FileGateway> gw;

    void SetUp() override
    {
        SoftwareContainerGatewayTest::SetUp();

        gw = std::unique_ptr<FileGateway>(new FileGateway(m_container));

        // Create files
        assert(mkdir(DIR_PATH.c_str(), 775) == 0);
        assert(createFile(FILE_PATH, FILE_CONTENT));
        assert(createFile(LONG_FILE_PATH, FILE_CONTENT_2));
        assert(createFile(FILE_PATH_W_SPACES, FILE_CONTENT_2));
    }

    void TearDown() override
    {
        // Remove file
        unlink(FILE_PATH.c_str());
        unlink(LONG_FILE_PATH.c_str());
        unlink(FILE_PATH_W_SPACES.c_str());
        rmdir(DIR_PATH.c_str());

        SoftwareContainerGatewayTest::TearDown();
    }

    bool createFile(const std::string &path, const std::string &content)
    {
        std::ofstream out(path);
        if (out.is_open()) {
            out << content;
            return out.good();
        }
        return false;
    }

    const std::string FILE_CONTENT = "ahdkhqweuyreqiwenomndlaskmd";
    const std::string FILE_PATH = "/tmp/filename.txt";
    const std::string DIR_PATH = "/tmp/testdir/";
    const std::string LONG_FILE_PATH = DIR_PATH + "filename.txt";
    const std::string FILE_CONTENT_2 = "bgsajbjkbjherkgbjhsbnjdfnjk";
    const std::string FILE_W_SPACES = "example file name.txt";
    const std::string FILE_PATH_W_SPACES = "/tmp/" + FILE_W_SPACES;
    const std::string CONTAINER_PATH = "/filename.txt";
};

/*
 * Test that a minimal conf is accepted and that activate works
 */
TEST_F(FileGatewayTest, ActivateWithMinimalValidConf) {
    const std::string config =
    "["
        "{"
            "  \"path-host\" : \"" + FILE_PATH + "\""
            ", \"path-container\" : \"" + CONTAINER_PATH + "\""
        "}"
    "]";
    loadConfig(config);
    ASSERT_TRUE(gw->setConfig(jsonConfig));
    ASSERT_TRUE(gw->activate());
}

/*
 * Test that bind mounting with a path which is malformed
 * with too many "/" does not cause a failure.
 */
TEST_F(FileGatewayTest, MalformedPath) {
    const std::string config =
        "[{"
        "  \"path-host\": \"//tmp//filename.txt\""
        ", \"path-container\": \"///filename.txt\""
        "}]";
    loadConfig(config);

    ASSERT_TRUE(gw->setConfig(jsonConfig));
    ASSERT_TRUE(gw->activate());
}

/*
 * Test that bind mounting with a path which does not exist
 * causes the bind mounting to fail.
 */
TEST_F(FileGatewayTest, MountNonExisting) {
    const std::string config =
        "[{"
            "  \"path-host\": \"/tmp/thisDirDoesNotExist/thisFileDoesNotExist.txt\""
            ", \"path-container\": \"/tmp/thisFileDoesNotExist.txt\""
        "}]";
    loadConfig(config);

    ASSERT_TRUE(gw->setConfig(jsonConfig));
    ASSERT_FALSE(gw->activate());
}

/*
 * Test that bind mounting with a destination path which is outside
 * the /gateway dir is not allowed.
 */
TEST_F(FileGatewayTest, MountOutsideContainerDir) {
    const std::string config =
        "[{"
            "  \"path-host\": \"" + FILE_PATH + "\""
            ", \"path-container\": \".." + CONTAINER_PATH + "\""
        "}]";
    loadConfig(config);

    ASSERT_TRUE(gw->setConfig(jsonConfig));
    ASSERT_FALSE(gw->activate());
}

/*
 * Test that bind mounting with a destination path which is the same
 * as one already created is not allowed.
 */
TEST_F(FileGatewayTest, MountSameFilename) {
    const std::string config =
        "[{"
            "  \"path-host\": \"" + FILE_PATH + "\""
            ", \"path-container\": \"" + CONTAINER_PATH + "\""
        "}]";
    loadConfig(config);

    EXPECT_TRUE(gw->setConfig(jsonConfig));

    const std::string config2 =
        "[{"
            "  \"path-host\": \"" + LONG_FILE_PATH + "\""
            ", \"path-container\": \"" + CONTAINER_PATH + "\""
        "}]";
    loadConfig(config2);

    // Throws GatewayError
    ASSERT_FALSE(gw->setConfig(jsonConfig));
}

/*
 * Test that bind mounting with a destination path which is a file
 * when the host path is a directory is not allowed.
 */
TEST_F(FileGatewayTest, MountDirAsFile) {
    const std::string config =
        "[{"
            "  \"path-host\": \"" + FILE_PATH + "\""
            ", \"path-container\": \"/app/testdir/\""
        "}]";
    loadConfig(config);

    ASSERT_TRUE(gw->setConfig(jsonConfig));
    ASSERT_FALSE(gw->activate());
}

/*
 * Test that it is possible to bind mount a file name with spaces.
 */
TEST_F(FileGatewayTest, MountPathWithSpaces) {
    const std::string FILE_W_SPACES = "example file name.txt";
    const std::string FILE_PATH_W_SPACES = "/tmp/example file name.txt";
    const std::string config =
        "[{"
            "  \"path-host\" : \"" + FILE_PATH + "\""
            ", \"path-container\" : \"/" + FILE_W_SPACES + "\""
        "}]";
    loadConfig(config);
    ASSERT_TRUE(gw->setConfig(jsonConfig));

    const std::string config2 =
        "[{"
            "  \"path-host\" : \"" + FILE_PATH_W_SPACES + "\""
            ", \"path-container\": \"" + CONTAINER_PATH + "\""
        "}]";
    loadConfig(config2);
    ASSERT_TRUE(gw->setConfig(jsonConfig));
    ASSERT_TRUE(gw->activate());
}
