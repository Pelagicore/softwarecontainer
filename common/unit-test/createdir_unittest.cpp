/*
 * Copyright (C) 2017 Pelagicore AB
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

#include "createdir.h"
#include "softwarecontainererror.h"

#include <gtest/gtest.h>
#include <sys/stat.h>

using namespace softwarecontainer;

class CreateDirTest: public ::testing::Test
{
public:

    void SetUp() override
    {
    }

    CreateDir cd;

};


TEST_F(CreateDirTest, createNestedDir)
{
    ASSERT_TRUE(cd.createDirectory("time/to/test/nested/directory/creation"));
    ASSERT_TRUE(cd.rollBack());
}

TEST_F(CreateDirTest, createExistingDir)
{
    ASSERT_TRUE(cd.createDirectory("test/double/creation"));
    ASSERT_TRUE(cd.createDirectory("test/double/creation"));
    ASSERT_TRUE(cd.rollBack());
}

TEST_F(CreateDirTest, DISABLED_creationError)
{
    ASSERT_TRUE(cd.createDirectory("/tmp/test/error/in/nested"));
    chmod("/tmp/test/error/in/nested", 444);
    ASSERT_FALSE(cd.createDirectory("/tmp/test/error/in/nested/directory/error"));
    ASSERT_TRUE(cd.rollBack());
}

TEST_F(CreateDirTest, tempError)
{
    EXPECT_THROW(cd.tempDir("tmp/sc-unappropriate/template"), SoftwareContainerError);
    ASSERT_TRUE(cd.rollBack());
}
