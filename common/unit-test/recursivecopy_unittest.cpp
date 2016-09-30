
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
#include <filetoolkitwithundo.h>
#include <recursivecopy.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <unistd.h>
#include <fstream>


class RecursiveCopyTest: public ::testing::Test
{
public:
    RecursiveCopyTest() { }

    void SetUp() override
    {
        srcdir = ft.tempDir("/tmp/sc-recursivecopyTest-XXXXXX");
        dstdir = ft.tempDir("/tmp/sc-recursivecopyTest-XXXXXX");
    }

    FileToolkitWithUndo ft;
    std::string srcdir;
    std::string dstdir;


};

void createFile(std::string dst, std::string val="abcdefg123")
{
    std::ofstream f;
    f.open(dst);
    f << val;
    f.close();
}

bool checkContent(std::string dst, std::string val)
{
    std::string content;
    std::ifstream f;
    f.open(dst);
    f >> content;
    f.close();
    return(content == val);
}

TEST_F(RecursiveCopyTest, copyDirEmptyToEmpty)
{
    ASSERT_TRUE(RecursiveCopy::getInstance().copy(srcdir, dstdir) == ReturnCode::SUCCESS);
    ASSERT_TRUE(isDirectoryEmpty(srcdir));
    ASSERT_TRUE(isDirectoryEmpty(dstdir));
}

TEST_F(RecursiveCopyTest, copyDirFilesToEmpty)
{
    createFile(srcdir + "/lala.txt");
    ASSERT_TRUE(RecursiveCopy::getInstance().copy(srcdir, dstdir) == ReturnCode::SUCCESS);
    ASSERT_TRUE(isFile(srcdir + "/lala.txt"));
    ASSERT_TRUE(isFile(dstdir + "/lala.txt"));
}

TEST_F(RecursiveCopyTest, copyDirEmptyToFiles)
{
    createFile(dstdir + "/lala.txt");
    ASSERT_TRUE(RecursiveCopy::getInstance().copy(srcdir, dstdir) == ReturnCode::SUCCESS);
    ASSERT_FALSE(isFile(srcdir + "/lala.txt"));
    ASSERT_TRUE(isFile(dstdir + "/lala.txt"));
    ASSERT_TRUE(isDirectoryEmpty(srcdir));
}

TEST_F(RecursiveCopyTest, copyDirFilesToFiles)
{
    createFile(srcdir + "/lala.txt");
    createFile(dstdir + "/lala2.txt");
    ASSERT_FALSE(isFile(srcdir + "/lala2.txt"));
    ASSERT_TRUE(RecursiveCopy::getInstance().copy(srcdir, dstdir) == ReturnCode::SUCCESS);
    ASSERT_TRUE(isFile(srcdir + "/lala.txt"));
    ASSERT_FALSE(isFile(srcdir + "/lala2.txt"));
    ASSERT_TRUE(isFile(dstdir + "/lala.txt"));
    ASSERT_TRUE(isFile(dstdir + "/lala2.txt"));
}

TEST_F(RecursiveCopyTest, copyDirFilesToFilesOverWrite)
{
    std::string content1 = "abcdefg123";
    std::string content2 = "123gfedcba";
    createFile(srcdir + "/lala.txt", content1);
    createFile(dstdir + "/lala.txt", content2);
    ASSERT_TRUE(checkContent(srcdir + "/lala.txt", content1));
    ASSERT_TRUE(checkContent(dstdir + "/lala.txt", content2));
    ASSERT_TRUE(RecursiveCopy::getInstance().copy(srcdir, dstdir) == ReturnCode::SUCCESS);
    ASSERT_TRUE(isFile(srcdir + "/lala.txt"));
    ASSERT_TRUE(isFile(dstdir + "/lala.txt"));
    ASSERT_TRUE(checkContent(srcdir + "/lala.txt", content1));
    ASSERT_TRUE(checkContent(dstdir + "/lala.txt", content1));
}

