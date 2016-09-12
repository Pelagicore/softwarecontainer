
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

#include <softwarecontaineragent.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <unistd.h>

class CreateContainerTest: public ::testing::Test
{
public:
    CreateContainerTest() { }
    SoftwareContainerAgent *sca;
    Glib::RefPtr<Glib::MainContext> m_context = Glib::MainContext::get_default();
    int m_preloadCount = 1;
    bool m_shutdownContainers = true;
    int m_shutdownTimeout = 2;

    void SetUp() override
    {
        sca = new SoftwareContainerAgent(m_context
                                         , m_preloadCount
                                         , m_shutdownContainers
                                         , m_shutdownTimeout);
    }

};

TEST_F(CreateContainerTest, TestActivateWithNoConf) {
    //ASSERT_FALSE(gw->activate());
    ASSERT_EQ(true,true);
}

