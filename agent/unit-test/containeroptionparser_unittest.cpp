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

#include "containeroptions/containeroptionparser.h"
#include "containeroptions/dynamiccontaineroptions.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace softwarecontainer;

class ContainerOptionParserTest : public ::testing::Test
{
public:
    LOG_DECLARE_CLASS_CONTEXT("TEST", "Tester");

    ContainerOptionParser m_parser;
    std::unique_ptr<DynamicContainerOptions> m_options;

    void SetUp() override
    {
        m_options = std::unique_ptr<DynamicContainerOptions>(new DynamicContainerOptions());
    }

    void parse(const std::string &config)
    {
        m_options = std::move(m_parser.parse(config));
    }
};

/*
 * Test a simple configuration and see that it's set to true
 */
TEST_F(ContainerOptionParserTest, parseConfigNiceEnabled) {
    ASSERT_NO_THROW(parse("[{\"writeBufferEnabled\": true}]"));
    ASSERT_TRUE(m_options->writeBufferEnabled());
}

/*
 * Test a simple configuration and make sure that it works multiple times
 */
TEST_F(ContainerOptionParserTest, parseManyTimes) {
    ASSERT_NO_THROW(parse("[{\"writeBufferEnabled\": true}]"));
    ASSERT_TRUE(m_options->writeBufferEnabled());
    ASSERT_NO_THROW(parse("[{\"writeBufferEnabled\": false}]"));
    ASSERT_FALSE(m_options->writeBufferEnabled());
    ASSERT_NO_THROW(parse("[{\"writeBufferEnabled\": true}]"));
    ASSERT_TRUE(m_options->writeBufferEnabled());
}

/*
 * Test that the configuration is disabled if it's set to being disabled
 */
TEST_F(ContainerOptionParserTest, parseConfigNiceDisabled) {
    ASSERT_NO_THROW(parse("[{\"writeBufferEnabled\": false}]"));
    ASSERT_FALSE(m_options->writeBufferEnabled());
}

/*
 * Parse a "good" configuration with all parts set in it
 */
TEST_F(ContainerOptionParserTest, parseConfigNiceWithTmpfsAndSize) {
    ASSERT_NO_THROW(parse("[{\"writeBufferEnabled\": true, \
                    \"temporaryFileSystemWriteBufferEnabled\": true, \
                    \"temporaryFileSystemSize\": 10485760}]"));
    ASSERT_TRUE(m_options->writeBufferEnabled());
    ASSERT_TRUE(m_options->temporaryFileSystemWriteBufferEnabled());
    ASSERT_EQ(m_options->temporaryFileSystemSize(), 10485760);
}

/*
 * Parse a "good" configuration with the enableTemporary.. disabled. Size
 * should not be parsed in that case.
 */
TEST_F(ContainerOptionParserTest, parseConfigNiceWithTmpfsDisabled) {
    ASSERT_NO_THROW(parse("[{\"writeBufferEnabled\": true, \
                    \"temporaryFileSystemWriteBufferEnabled\": false, \
                    \"temporaryFileSystemSize\": 10485760}]"));
    ASSERT_TRUE(m_options->writeBufferEnabled());
    ASSERT_FALSE(m_options->temporaryFileSystemWriteBufferEnabled());
}

/*
 * Parse a "good" configuration with a smaller size.
 */
TEST_F(ContainerOptionParserTest, parseConfigNiceTmpfsEnabledSmall) {
    ASSERT_NO_THROW(parse("[{\"writeBufferEnabled\": true, \
                    \"temporaryFileSystemWriteBufferEnabled\": true, \
                    \"temporaryFileSystemSize\": 100}]"));
    ASSERT_TRUE(m_options->writeBufferEnabled());
    ASSERT_TRUE(m_options->temporaryFileSystemWriteBufferEnabled());
    ASSERT_EQ(m_options->temporaryFileSystemSize(), 100);
}

/*
 * Parse a "good" configuration where everything is disabled.
 */
TEST_F(ContainerOptionParserTest, parseConfigNiceDisabledWithExtraConfig) {
    ASSERT_NO_THROW(parse("[{\"writeBufferEnabled\": false, \
                    \"temporaryFileSystemWriteBufferEnabled\": true, \
                    \"temporaryFileSystemSize\": 100}]"));
    ASSERT_FALSE(m_options->writeBufferEnabled());
    ASSERT_FALSE(m_options->temporaryFileSystemWriteBufferEnabled());
}

/*
 * Parse a config with no config
 */
TEST_F(ContainerOptionParserTest, parseConfigNoConfig) {
    ASSERT_THROW(parse(""), ContainerOptionParseError);
}

/*
 *  Parse a badly formatted config
 */
TEST_F(ContainerOptionParserTest, parseConfigBadConfig) {
    ASSERT_THROW(parse("gobfmsrfe"), ContainerOptionParseError);
}

/*
 * Parse a config with a missing Size parameter and make sure it still
 * parses.
 */
TEST_F(ContainerOptionParserTest, parseConfigNiceConfigMissingSize) {
    ASSERT_NO_THROW(parse("[{\"writeBufferEnabled\": true, \
                    \"temporaryFileSystemWriteBufferEnabled\": true}]"));
    ASSERT_TRUE(m_options->writeBufferEnabled());
    ASSERT_TRUE(m_options->temporaryFileSystemWriteBufferEnabled());
}

/*
 * Parse a "good" config where the tmpfs is disabled and missing size.
 */
TEST_F(ContainerOptionParserTest, parseConfigNiceConfigTmpfsDisabledMissingSize) {
    ASSERT_NO_THROW(parse("[{\"writeBufferEnabled\": true, \
                    \"temporaryFileSystemWriteBufferEnabled\": false}]"));
    ASSERT_TRUE(m_options->writeBufferEnabled());
    ASSERT_FALSE(m_options->temporaryFileSystemWriteBufferEnabled());
}

/*
 * Parse a "good" config where enableTemporary... and size is missing
 */
TEST_F(ContainerOptionParserTest, parseConfigNiceConfigMissingTmpfSConfig) {
    ASSERT_NO_THROW(parse("[{\"writeBufferEnabled\": true}]"));
    ASSERT_TRUE(m_options->writeBufferEnabled());
}

/*
 * Parse a "bad" config where there is a bad parameter name.
 */
TEST_F(ContainerOptionParserTest, parseConfigEvilConfig) {
    ASSERT_THROW(parse("[{\"WRONG_PARAM_NAME\": true}]"), ContainerOptionParseError);
    ASSERT_FALSE(m_options->writeBufferEnabled());
}

