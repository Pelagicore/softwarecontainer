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
#include "softwarecontainer-common.h"

#include <gtest/gtest.h>
#include <sys/stat.h>

using namespace softwarecontainer;

/*
 * Verify creation of nested directories will not create an error
 */
TEST(CreateDirTest, createNestedDir)
{
    {
        CreateDir cd;
        ASSERT_TRUE(cd.createDirectory("time/to/test/nested/directory/creation"));
    }
    ASSERT_FALSE(isDirectory("time/to/test/nested/directory/creation"));
}

/*
 * Verify creation of existing directory will not create an error
 */
TEST(CreateDirTest, createExistingDir)
{
    {
        CreateDir cd;
        ASSERT_TRUE(cd.createDirectory("test/double/creation"));
        ASSERT_TRUE(cd.createDirectory("test/double/creation"));
    }
    ASSERT_FALSE(isDirectory("test/double/creation"));
}

/*
 * Verify rollback mechanism works correctly
 * This test is disabled due to current circumstances force unit tests to be run by root user
 * It will be enabled again when that necessity is removed.
 */
TEST(CreateDirTest, DISABLED_creationError)
{
    {
        CreateDir cd;
        ASSERT_TRUE(cd.createDirectory("/tmp/test/error/in/nested"));
        chmod("/tmp/test/error/in/nested", 444);
        ASSERT_FALSE(cd.createDirectory("/tmp/test/error/in/nested/directory/error"));
    }
    ASSERT_FALSE(isDirectory("/tmp/test/error/in/nested"));
}

/*
 * Verify tempdir function works correctly
 */
TEST(CreateDirTest, tempError)
{
    {
        CreateDir cd;
        EXPECT_THROW(cd.createTempDirectoryFromTemplate("tmp/sc-unappropriate/template"), SoftwareContainerError);
    }
    ASSERT_FALSE(isDirectory("tmp/sc-unappropriate/template"));
}
