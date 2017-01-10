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

#include <softwarecontainer-common.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

using namespace softwarecontainer;

class SoftwareContainerCommonTest: public ::testing::Test
{
public:
    SoftwareContainerCommonTest() { }

    void SetUp() override {
    }

    /*
     * Create a temporary directory or file, and optionally remove it.
     * Removal is useful if one only wants a unique tmp name
     */
    std::string getTempPath(bool directory, bool shouldUnlink)
    {
        char fileTemplate[] = "/tmp/SC-tmpXXXXXX";
        int fd = 0;

        if (directory) {
            mkdtemp(fileTemplate);
            if (shouldUnlink) {
                rmdir(fileTemplate);
            }
        } else {
            fd = mkstemp(fileTemplate);
            close(fd);
            if (shouldUnlink) {
                unlink(fileTemplate);
            }
        }

        return std::string(fileTemplate);
    }
};

TEST_F(SoftwareContainerCommonTest, ReturnCodeOperatorEquals) {
    ASSERT_TRUE(ReturnCode::SUCCESS == ReturnCode::SUCCESS);
    ASSERT_FALSE(ReturnCode::FAILURE == ReturnCode::SUCCESS);
    ASSERT_FALSE(ReturnCode::SUCCESS == ReturnCode::FAILURE);
    ASSERT_TRUE(ReturnCode::FAILURE == ReturnCode::FAILURE);
}

TEST_F(SoftwareContainerCommonTest, bool2ReturnCode) {
    ASSERT_TRUE(bool2ReturnCode(true) == ReturnCode::SUCCESS);
    ASSERT_TRUE(bool2ReturnCode(false) == ReturnCode::FAILURE);
}

TEST_F(SoftwareContainerCommonTest, ReturnCodeisError) {
    ASSERT_TRUE(isError(ReturnCode::SUCCESS) == false);
    ASSERT_TRUE(isError(ReturnCode::FAILURE) == true);
}

TEST_F(SoftwareContainerCommonTest, ReturnCodeisSuccess) {
    ASSERT_TRUE(isSuccess(ReturnCode::SUCCESS) == true);
    ASSERT_TRUE(isSuccess(ReturnCode::FAILURE) == false);
}

/*
 * Just a sample of bad paths, to see that it is being handled properly.
 */
TEST_F(SoftwareContainerCommonTest, BadPathsAreInvalid) {
    std::string badPaths[] = {
        "/tmp/blaha/////",
        "/tmp/\n",
        "/tmp/\"",
    };

    for (std::string badPath : badPaths) {
        ASSERT_FALSE(existsInFileSystem(badPath));
        ASSERT_FALSE(isDirectory(badPath));
        ASSERT_FALSE(isFile(badPath));
        ASSERT_FALSE(isPipe(badPath));
        ASSERT_FALSE(isSocket(badPath));
    }
}

/*
 * Test that a directory is only seen as a directory and not as a file, socket etc.
 */
TEST_F(SoftwareContainerCommonTest, DirIsOnlyDir) {
    bool createDir = true;
    bool shouldUnlink = false;
    std::string dirStr = getTempPath(createDir, shouldUnlink);

    ASSERT_TRUE(existsInFileSystem(dirStr));
    ASSERT_TRUE(isDirectory(dirStr));
    ASSERT_FALSE(isFile(dirStr));
    ASSERT_FALSE(isPipe(dirStr));
    ASSERT_FALSE(isSocket(dirStr));

    ASSERT_EQ(rmdir(dirStr.c_str()), 0);

    ASSERT_FALSE(existsInFileSystem(dirStr));
    ASSERT_FALSE(isDirectory(dirStr));
    ASSERT_FALSE(isFile(dirStr));
    ASSERT_FALSE(isPipe(dirStr));
    ASSERT_FALSE(isSocket(dirStr));
}

/*
 * Test that a regular file is only recognized as a file and not as a dir, socket etc.
 */
TEST_F(SoftwareContainerCommonTest, FileIsOnlyFile) {
    bool createDir = false;
    bool shouldUnlink = false;
    std::string fileStr = getTempPath(createDir, shouldUnlink);

    ASSERT_TRUE(existsInFileSystem(fileStr));
    ASSERT_TRUE(isFile(fileStr));
    ASSERT_FALSE(isDirectory(fileStr));
    ASSERT_FALSE(isPipe(fileStr));
    ASSERT_FALSE(isSocket(fileStr));

    // Cleanup
    ASSERT_EQ(unlink(fileStr.c_str()), 0);

    ASSERT_FALSE(existsInFileSystem(fileStr));
    ASSERT_FALSE(isFile(fileStr));
    ASSERT_FALSE(isDirectory(fileStr));
    ASSERT_FALSE(isPipe(fileStr));
    ASSERT_FALSE(isSocket(fileStr));
}

/*
 * Test that a socket is seen as a socket, but not as a file or dir etc
 */
TEST_F(SoftwareContainerCommonTest, SocketIsOnlySocket) {
    bool createDir = false;
    bool shouldUnlink = true;
    std::string fileStr = getTempPath(createDir, shouldUnlink);

    // Create a socket
    int fd = socket(AF_UNIX, SOCK_DGRAM, 0);
    ASSERT_NE(fd, -1);

    // And bind it to the filepath
    struct sockaddr_un namesock;
    namesock.sun_family = AF_UNIX;
    strncpy(namesock.sun_path, fileStr.c_str(), sizeof(namesock.sun_path));
    ASSERT_EQ(bind(fd, (struct sockaddr *) &namesock, sizeof(struct sockaddr_un)), 0);
    ASSERT_EQ(close(fd), 0);

    ASSERT_TRUE(existsInFileSystem(fileStr));
    ASSERT_TRUE(isSocket(fileStr));
    ASSERT_FALSE(isPipe(fileStr));
    ASSERT_FALSE(isFile(fileStr));
    ASSERT_FALSE(isDirectory(fileStr));

    // Cleanup
    ASSERT_EQ(unlink(fileStr.c_str()), 0);

    ASSERT_FALSE(existsInFileSystem(fileStr));
    ASSERT_FALSE(isSocket(fileStr));
    ASSERT_FALSE(isPipe(fileStr));
    ASSERT_FALSE(isFile(fileStr));
    ASSERT_FALSE(isDirectory(fileStr));
}

/*
 * Test that a pipe is seen only as a pipe and not as a socket, file or dir.
 */
TEST_F(SoftwareContainerCommonTest, PipeIsOnlyPipe) {
    bool createDir = false;
    bool shouldUnlink = true;
    std::string pipeStr = getTempPath(createDir, shouldUnlink);
    ASSERT_FALSE(existsInFileSystem(pipeStr));

    // Create a pipe
    ASSERT_EQ(mknod(pipeStr.c_str(), S_IFIFO|0666, 0), 0);

    ASSERT_TRUE(existsInFileSystem(pipeStr));
    ASSERT_TRUE(isPipe(pipeStr));
    ASSERT_FALSE(isFile(pipeStr));
    ASSERT_FALSE(isDirectory(pipeStr));
    ASSERT_FALSE(isSocket(pipeStr));

    // Cleanup
    ASSERT_EQ(unlink(pipeStr.c_str()), 0);

    ASSERT_FALSE(existsInFileSystem(pipeStr));
    ASSERT_FALSE(isPipe(pipeStr));
    ASSERT_FALSE(isFile(pipeStr));
    ASSERT_FALSE(isDirectory(pipeStr));
    ASSERT_FALSE(isSocket(pipeStr));
}
