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

TEST_F(ContainerOptionParserTest, parseConfigNice) {
    ASSERT_NO_THROW(parse("[{\"enableWriteBuffer\": true}]"));
    ASSERT_TRUE(m_options->enableWriteBuffer());
}

TEST_F(ContainerOptionParserTest, parseManyTimes) {
    ASSERT_NO_THROW(parse("[{\"enableWriteBuffer\": true}]"));
    ASSERT_TRUE(m_options->enableWriteBuffer());
    ASSERT_NO_THROW(parse("[{\"enableWriteBuffer\": false}]"));
    ASSERT_FALSE(m_options->enableWriteBuffer());
    ASSERT_NO_THROW(parse("[{\"enableWriteBuffer\": true}]"));
    ASSERT_TRUE(m_options->enableWriteBuffer());
}

TEST_F(ContainerOptionParserTest, parseConfigNice2) {
    ASSERT_NO_THROW(parse("[{\"enableWriteBuffer\": false}]"));
    ASSERT_FALSE(m_options->enableWriteBuffer());
}

TEST_F(ContainerOptionParserTest, parseConfigNoConfig) {
    ASSERT_THROW(parse(""), ContainerOptionParseError);
}

TEST_F(ContainerOptionParserTest, parseConfigBadConfig) {
    ASSERT_THROW(parse("gobfmsrfe"), ContainerOptionParseError);
}

TEST_F(ContainerOptionParserTest, parseConfigEvilConfig) {
    ASSERT_THROW(parse("[{\"WRONG_PARAM_NAME\": true}]"), ContainerOptionParseError);
    ASSERT_FALSE(m_options->enableWriteBuffer());
}

