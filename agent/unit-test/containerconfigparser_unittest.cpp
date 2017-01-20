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

#include "containerconfigparser.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace softwarecontainer;

class ContainerConfigParserTest : public ::testing::Test
{
public:
    LOG_DECLARE_CLASS_CONTEXT("TEST", "Tester");

    ContainerConfigParser m_parser;
    ContainerConfigParser::ContainerOptions m_options;

    void parse(const std::string &config)
    {
        m_options = m_parser.parse(config);
    }
};

TEST_F(ContainerConfigParserTest, parseConfigNice) {
    ASSERT_NO_THROW(parse("[{\"enableWriteBuffer\": true}]"));
    ASSERT_TRUE(m_options.enableWriteBuffer);
}

TEST_F(ContainerConfigParserTest, parseConfigNice2) {
    ASSERT_NO_THROW(parse("[{\"enableWriteBuffer\": false}]"));
    ASSERT_FALSE(m_options.enableWriteBuffer);
}

TEST_F(ContainerConfigParserTest, parseConfigNoConfig) {
    ASSERT_THROW(parse(""), ContainerConfigParseError);
}

TEST_F(ContainerConfigParserTest, parseConfigBadConfig) {
    ASSERT_THROW(parse("gobfmsrfe"), ContainerConfigParseError);
}

TEST_F(ContainerConfigParserTest, parseConfigEvilConfig) {
    ASSERT_THROW(parse("[{\"WRONG_PARAM_NAME\": true}]"), ContainerConfigParseError);
    ASSERT_FALSE(m_options.enableWriteBuffer);
}

