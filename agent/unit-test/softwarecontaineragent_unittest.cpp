
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

class SoftwareContainerAgentTest: public ::testing::Test
{
public:
    LOG_DECLARE_CLASS_CONTEXT("TEST", "Tester");
    SoftwareContainerAgentTest() { }
    SoftwareContainerAgent *sca;
    Glib::RefPtr<Glib::MainContext> m_context = Glib::MainContext::get_default();
    int m_preloadCount = 1;
    bool m_shutdownContainers = true;
    int m_shutdownTimeout = 2;
    std::shared_ptr<Workspace> workspace;

    void SetUp() override
    {
        try {
            sca = new SoftwareContainerAgent(
                m_context
                , m_preloadCount
                , m_shutdownContainers
                , m_shutdownTimeout);

            workspace = sca->getWorkspace();
        } catch(ReturnCode failure) {
            log_error() << "Exception in software agent constructor";
            ASSERT_TRUE(false);
        }
    }

    void TearDown() override
    {
        delete sca;
    }
};

TEST_F(SoftwareContainerAgentTest, CreatAndCheckContainer) {
    SoftwareContainer *container;
    ContainerID id = sca->createContainer("iejr-", "");
    bool retval = sca->checkContainer(id, container);
    ASSERT_TRUE(retval == true);
}

TEST_F(SoftwareContainerAgentTest, DeleteContainer) {
    SoftwareContainer *container;
    ContainerID id = sca->createContainer("iejr-", "");
    sca->deleteContainer(id);
    bool retval = sca->checkContainer(id, container);
    ASSERT_TRUE(retval == false);
}

/*
 *TBD: This test needs to be fixed, somethings going on in it.
TEST_F(SoftwareContainerAgentTest, CreateContainerWithConf) {
    log_error() << "gobbles1";
    ContainerID id = sca->createContainer("iejr-", "[{\"enableWriteBuffer\": true}]");
    // This is actually only true if no other containers have been created
    // before this one. Might need to be fixed somehow.
    log_error() << "gobbles2";
    ASSERT_TRUE(id == 0);
    ASSERT_TRUE(workspace->m_enableWriteBuffer == true);
}
 */

TEST_F(SoftwareContainerAgentTest, parseConfigNice) {
    bool retval = sca->parseConfig("[{\"enableWriteBuffer\": true}]");
    ASSERT_TRUE(retval == true);
    ASSERT_TRUE(workspace->m_enableWriteBuffer == true);
}

TEST_F(SoftwareContainerAgentTest, parseConfigNice2) {
    bool retval = sca->parseConfig("[{\"enableWriteBuffer\": false}]");
    ASSERT_TRUE(retval == true);
}

TEST_F(SoftwareContainerAgentTest, parseConfigNoConfig) {
    bool retval = sca->parseConfig("");
    ASSERT_TRUE(retval== false);
}

TEST_F(SoftwareContainerAgentTest, parseConfigBadConfig) {
    bool retval = sca->parseConfig("gobfmsrfe");
    ASSERT_TRUE(retval== false);
}

TEST_F(SoftwareContainerAgentTest, parseConfigEvilConfig) {
    // This actually parses and should be true. but wrong parameter name
    bool retval = sca->parseConfig("[{\"enableWritebuffer\": true}]");
    ASSERT_TRUE(retval== true);
    ASSERT_TRUE(workspace->m_enableWriteBuffer == false);
}

TEST_F(SoftwareContainerAgentTest, parseConfigEvilConfig2) {
    // This actually parses and should be true
    bool retval = sca->parseConfig("[{\"enableWriteBuffer\": false}]");

    ASSERT_TRUE(retval == true);
    ASSERT_TRUE(workspace->m_enableWriteBuffer == false);
}
