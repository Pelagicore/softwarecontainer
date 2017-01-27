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
};

/*
 * Test that a minimal conf is accepted and that activate works
 */
TEST_F(FileGatewayTest, TestActivateWithMinimalValidConf) {
    givenContainerIsSet(gw);
    const std::string config =
    "["
        "{"
            "  \"path-host\" : \"" + FILE_PATH + "\""
            ", \"path-container\" : \"" + CONTAINER_PATH + "\""
        "}"
    "]";
    loadConfig(config);
    ASSERT_TRUE(isSuccess(gw->setConfig(jsonConfig)));
    ASSERT_TRUE(isSuccess(gw->activate()));
}
