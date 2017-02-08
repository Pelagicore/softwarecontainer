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

/*
 * Test that writing to a file and then reading
 * from it gives back the written data.
 */
TEST_F(SoftwareContainerCommonTest, ReadBackWrittenFile) {
    bool createDir = false;
    bool shouldUnlink = false;
    std::string fileName = getTempPath(createDir, shouldUnlink);

    std::string testData = "testData";
    std::string readBack;

    ASSERT_TRUE(writeToFile(fileName, testData));
    ASSERT_TRUE(readFromFile(fileName, readBack));
    ASSERT_EQ(testData, readBack);
}

/*
 * Test that reading from an nonexisting file won't work, but that
 * writing to a nonexisting file will create it.
 */
TEST_F(SoftwareContainerCommonTest, ReadWriteUnexistingFile) {
    bool createDir = false;
    bool shouldUnlink = true;
    std::string fileName = getTempPath(createDir, shouldUnlink);

    std::string testData = "testData";
    std::string readBack;

    ASSERT_FALSE(existsInFileSystem(fileName));
    ASSERT_FALSE(readFromFile(fileName, readBack));

    ASSERT_FALSE(existsInFileSystem(fileName));
    ASSERT_TRUE(writeToFile(fileName, testData));
    ASSERT_TRUE(existsInFileSystem(fileName));
}

/*
 * Test that touching a file will create it, but leave it empty
 */
TEST_F(SoftwareContainerCommonTest, TouchCreatesFiles) {
    bool createDir = false;
    bool shouldUnlink = true;
    std::string fileName = getTempPath(createDir, shouldUnlink);

    ASSERT_FALSE(existsInFileSystem(fileName));
    ASSERT_TRUE(touch(fileName));
    ASSERT_TRUE(existsInFileSystem(fileName));

    std::string readBack;
    ASSERT_TRUE(readFromFile(fileName, readBack));
    ASSERT_EQ(readBack, "");
}

/*
 * Test that touching a file won't modify its content, if it already
 * existed.
 */
TEST_F(SoftwareContainerCommonTest, TouchDoesntModifyFiles) {
    bool createDir = false;
    bool shouldUnlink = true;
    std::string fileName = getTempPath(createDir, shouldUnlink);

    std::string testData = "testData";
    std::string readBack;

    ASSERT_TRUE(writeToFile(fileName, testData));
    ASSERT_TRUE(readFromFile(fileName, readBack));
    ASSERT_EQ(testData, readBack);

    ASSERT_TRUE(touch(fileName));

    ASSERT_TRUE(readFromFile(fileName, readBack));
    ASSERT_EQ(testData, readBack);

}

TEST_F(SoftwareContainerCommonTest, ParentPathWorksAsExpected) {
    // Absolute paths
    EXPECT_EQ(parentPath("/"), "/");
    EXPECT_EQ(parentPath("/usr/"), "/");
    EXPECT_EQ(parentPath("/usr//"), "/");
    EXPECT_EQ(parentPath("/usr/lib"), "/usr");
    EXPECT_EQ(parentPath("/usr/lib/"), "/usr");
    EXPECT_EQ(parentPath("/usr/local/etc/some-file.txt"), "/usr/local/etc");

    // Relative paths
    EXPECT_EQ(parentPath(""), ".");
    EXPECT_EQ(parentPath("home"), ".");
    EXPECT_EQ(parentPath("home/test"), "home");
    EXPECT_EQ(parentPath("home/test/"), "home");
    EXPECT_EQ(parentPath("home/test/some-file.txt"), "home/test");
}

TEST_F(SoftwareContainerCommonTest, BaseNameWorksAsExpected) {
    EXPECT_EQ(baseName("/"), "/");
    EXPECT_EQ(baseName("/usr/"), "usr");
    EXPECT_EQ(baseName("/usr"), "usr");
    EXPECT_EQ(baseName("/usr/lib/"), "lib");
    EXPECT_EQ(baseName("/usr/lib"), "lib");
}
