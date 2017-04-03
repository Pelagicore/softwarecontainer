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
#include <filetoolkitwithundo.h>
#include <recursivecopy.h>

#include <gtest/gtest.h>
#include <unistd.h>
#include <fstream>

#include "unittest_common_helpers.h"

using namespace softwarecontainer;

class RecursiveCopyTest: public ::testing::Test, CreateDir
{
public:
    RecursiveCopyTest() { }

    void SetUp() override
    {
        srcdir = cd.createTempDirectoryFromTemplate("/tmp/sc-recursivecopyTest-XXXXXX");
        dstdir = cd.createTempDirectoryFromTemplate("/tmp/sc-recursivecopyTest-XXXXXX");
    }

    CreateDir cd;
    std::string srcdir;
    std::string dstdir;
};

TEST_F(RecursiveCopyTest, copyDirEmptyToEmpty)
{
    ASSERT_TRUE(RecursiveCopy::getInstance().copy(srcdir, dstdir));
    ASSERT_TRUE(isDirectoryEmpty(srcdir));
    ASSERT_TRUE(isDirectoryEmpty(dstdir));
}

TEST_F(RecursiveCopyTest, copyDirFilesToEmpty)
{
    std::string srcFile = buildPath(srcdir, "lala.txt");
    std::string dstFile = buildPath(dstdir, "lala.txt");
    createFile(srcFile);

    ASSERT_TRUE(RecursiveCopy::getInstance().copy(srcdir, dstdir));
    ASSERT_TRUE(isFile(srcFile));
    ASSERT_TRUE(isFile(dstFile));
}

TEST_F(RecursiveCopyTest, copyDirEmptyToFiles)
{
    std::string dstFile = buildPath(dstdir, "lala.txt");
    std::string srcFile = buildPath(srcdir, "lala.txt");
    createFile(dstFile);

    ASSERT_TRUE(RecursiveCopy::getInstance().copy(srcdir, dstdir));
    ASSERT_FALSE(isFile(srcFile));
    ASSERT_TRUE(isFile(dstFile));
    ASSERT_TRUE(isDirectoryEmpty(srcdir));
}

TEST_F(RecursiveCopyTest, copyDirFilesToFiles)
{
    std::string srcFile = buildPath(srcdir, "lala.txt");
    std::string srcFile2 = buildPath(srcdir, "lala2.txt");
    std::string dstFile = buildPath(dstdir, "lala.txt");
    std::string dstFile2 = buildPath(dstdir, "lala2.txt");
    createFile(srcFile);
    createFile(dstFile2);

    ASSERT_FALSE(isFile(srcFile2));

    ASSERT_TRUE(RecursiveCopy::getInstance().copy(srcdir, dstdir));

    ASSERT_TRUE(isFile(srcFile));
    ASSERT_FALSE(isFile(srcFile2));

    ASSERT_TRUE(isFile(dstFile));
    ASSERT_TRUE(isFile(dstFile2));
}

TEST_F(RecursiveCopyTest, copyDirFilesToFilesOverWrite)
{
    std::string content1 = "abcdefg123";
    std::string content2 = "123gfedcba";

    std::string srcFile = buildPath(srcdir, "lala.txt");
    std::string dstFile = buildPath(dstdir, "lala.txt");

    createFile(srcFile, content1);
    createFile(dstFile, content2);

    ASSERT_TRUE(checkContent(srcFile, content1));
    ASSERT_TRUE(checkContent(dstFile, content2));

    ASSERT_TRUE(RecursiveCopy::getInstance().copy(srcdir, dstdir));

    ASSERT_TRUE(isFile(srcFile));
    ASSERT_TRUE(isFile(dstFile));

    ASSERT_TRUE(checkContent(srcFile, content1));
    ASSERT_TRUE(checkContent(dstFile, content1));
}
