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

#include "gateway/files/filegatewaysettingstore.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

using namespace softwarecontainer;

class FileGatewaySettingStoreTest : public ::testing::Test
{
public:
    FileGatewaySettingStore store;

    const std::string FILE_PATH = "/tmp/filename.txt";
    const std::string OTHER_FILE_PATH = "/tmp/whoops.txt";
    const std::string CONTAINER_PATH = "/tmp/emanelif.txt";
    const std::string OTHER_CONTAINER_PATH = "/tmp/spoohw.txt";
};

/*
 * Using the same configuration twice is ok, as long as the container path
 * is the same. The two configs will be merged, so size will not increase
 */
TEST_F(FileGatewaySettingStoreTest, SameConfigTwice) {

    FileGatewayParser::FileSetting setting;
    setting.pathInHost = FILE_PATH;
    setting.pathInContainer = CONTAINER_PATH;
    setting.readOnly = true;

    ASSERT_TRUE(store.addSetting(setting));
    ASSERT_EQ(1u, store.getSettings().size());

    ASSERT_TRUE(store.addSetting(setting));
    ASSERT_EQ(1u, store.getSettings().size());
}

/*
 * Using the same paths but with different values on read-only is fine, since we
 * will use the most permissive setting.
 */
TEST_F(FileGatewaySettingStoreTest, ReadOnlyPrecedence) {

    FileGatewayParser::FileSetting settingReadOnly;
    settingReadOnly.pathInHost = FILE_PATH;
    settingReadOnly.pathInContainer = CONTAINER_PATH;
    settingReadOnly.readOnly = true;

    FileGatewayParser::FileSetting settingReadWrite;
    settingReadWrite.pathInHost = FILE_PATH;
    settingReadWrite.pathInContainer = CONTAINER_PATH;
    settingReadWrite.readOnly = false;

    ASSERT_TRUE(store.addSetting(settingReadOnly));
    ASSERT_EQ(1u, store.getSettings().size());

    ASSERT_TRUE(store.addSetting(settingReadWrite));
    ASSERT_EQ(1u, store.getSettings().size());

    ASSERT_EQ(store.getSettings().at(0).readOnly, false);
}

/*
 * Testing same host path but different container paths is fine, we will
 * mount the same file to two places, no problems.
 */
TEST_F(FileGatewaySettingStoreTest, SameHostPathDifferentContainerPath) {

    FileGatewayParser::FileSetting settingOne;
    settingOne.pathInHost = FILE_PATH;
    settingOne.pathInContainer = CONTAINER_PATH;

    FileGatewayParser::FileSetting settingTwo;
    settingTwo.pathInHost = FILE_PATH;
    settingTwo.pathInContainer = OTHER_CONTAINER_PATH;

    ASSERT_TRUE(store.addSetting(settingOne));
    ASSERT_EQ(1u, store.getSettings().size());

    ASSERT_TRUE(store.addSetting(settingTwo));
    ASSERT_EQ(2u, store.getSettings().size());
}

/*
 * Testing different host paths to the same container path. This is not OK, since
 * there is no way to know which file is desired to have in the container.
 */
TEST_F(FileGatewaySettingStoreTest, DifferentHostPathSameContainerPath) {
    FileGatewayParser::FileSetting settingOne;
    settingOne.pathInHost = FILE_PATH;
    settingOne.pathInContainer = CONTAINER_PATH;

    FileGatewayParser::FileSetting settingTwo;
    settingTwo.pathInHost = OTHER_FILE_PATH;
    settingTwo.pathInContainer = CONTAINER_PATH;

    ASSERT_TRUE(store.addSetting(settingOne));
    ASSERT_EQ(1u, store.getSettings().size());

    ASSERT_FALSE(store.addSetting(settingTwo));
    ASSERT_EQ(1u, store.getSettings().size());
}
