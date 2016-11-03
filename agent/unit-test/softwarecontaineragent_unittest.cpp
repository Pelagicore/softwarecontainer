
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
#include <memory>

class SoftwareContainerAgentTest: public ::testing::Test
{
public:
    LOG_DECLARE_CLASS_CONTEXT("TEST", "Tester");
    SoftwareContainerAgentTest() { }
    std::shared_ptr<SoftwareContainerAgent> sca;
    Glib::RefPtr<Glib::MainContext> m_context = Glib::MainContext::get_default();
    int m_preloadCount = 0;
    bool m_shutdownContainers = true;
    int m_shutdownTimeout = 2;
    std::shared_ptr<Workspace> workspace;

    const std::string valid_config = "[{\"enableWriteBuffer\": false}]";

    void SetUp() override
    {
        try {
            sca = std::make_shared<SoftwareContainerAgent>(
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
    }
};

TEST_F(SoftwareContainerAgentTest, CreatAndCheckContainer) {
    SoftwareContainer *container;
    ContainerID id = sca->createContainer(valid_config);
    bool retval = sca->checkContainer(id, container);
    ASSERT_TRUE(retval);
}

TEST_F(SoftwareContainerAgentTest, DeleteContainer) {
    SoftwareContainer *container;
    ContainerID id = sca->createContainer(valid_config);
    sca->deleteContainer(id);
    bool retval = sca->checkContainer(id, container);
    ASSERT_FALSE(retval);
}

/*
 *TBD: This test needs to be fixed, somethings going on in it.
TEST_F(SoftwareContainerAgentTest, CreateContainerWithConf) {
    log_error() << "gobbles1";
    ContainerID id = sca->createContainer("[{\"enableWriteBuffer\": true}]");
    // This is actually only true if no other containers have been created
    // before this one. Might need to be fixed somehow.
    log_error() << "gobbles2";
    ASSERT_TRUE(id == 0);
    ASSERT_TRUE(workspace->m_enableWriteBuffer == true);
}
 */

TEST_F(SoftwareContainerAgentTest, parseConfigNice) {
    bool retval = sca->parseConfig("[{\"enableWriteBuffer\": true}]");
    ASSERT_TRUE(retval);
    ASSERT_TRUE(workspace->m_enableWriteBuffer);
}

TEST_F(SoftwareContainerAgentTest, parseConfigNice2) {
    bool retval = sca->parseConfig("[{\"enableWriteBuffer\": false}]");
    ASSERT_TRUE(retval);
}

TEST_F(SoftwareContainerAgentTest, parseConfigNoConfig) {
    bool retval = sca->parseConfig("");
    ASSERT_FALSE(retval);
}

TEST_F(SoftwareContainerAgentTest, parseConfigBadConfig) {
    bool retval = sca->parseConfig("gobfmsrfe");
    ASSERT_FALSE(retval);
}

TEST_F(SoftwareContainerAgentTest, parseConfigEvilConfig) {
    // This actually parses and should be true. but wrong parameter name
    bool retval = sca->parseConfig("[{\"enableWritebuffer\": true}]");
    ASSERT_TRUE(retval);
    ASSERT_FALSE(workspace->m_enableWriteBuffer);
}

TEST_F(SoftwareContainerAgentTest, parseConfigEvilConfig2) {
    // This actually parses and should be true
    bool retval = sca->parseConfig("[{\"enableWriteBuffer\": false}]");

    ASSERT_TRUE(retval);
    ASSERT_FALSE(workspace->m_enableWriteBuffer);
}

// Freeze an invalid container
TEST_F(SoftwareContainerAgentTest, FreezeInvalidContainer) {
    bool retval = sca->suspendContainer(0);
    ASSERT_FALSE(retval);
}

// Thaw an invalid container
TEST_F(SoftwareContainerAgentTest, ThawInvalidContainer) {
    bool retval = sca->resumeContainer(0);
    ASSERT_FALSE(retval);
}

// Thaw a container that has not been frozen
TEST_F(SoftwareContainerAgentTest, ThawUnfrozenContainer) {
    SoftwareContainer *container;
    ContainerID id = sca->createContainer(valid_config);
    bool retval = sca->checkContainer(id, container);
    ASSERT_TRUE(retval);

    retval = sca->resumeContainer(id);
    ASSERT_FALSE(retval);
}

// Freeze and then resume a container
TEST_F(SoftwareContainerAgentTest, FreezeAndThawContainer) {
    SoftwareContainer *container;
    ContainerID id = sca->createContainer(valid_config);
    bool retval = sca->checkContainer(id, container);
    ASSERT_TRUE(retval);

    retval = sca->suspendContainer(id);
    ASSERT_TRUE(retval);

    retval = sca->resumeContainer(id);
    ASSERT_TRUE(retval);
}

// Freeze a container and try to resume it twice
TEST_F(SoftwareContainerAgentTest, FreezeContainerAndThawTwice) {
    SoftwareContainer *container;
    ContainerID id = sca->createContainer(valid_config);
    bool retval = sca->checkContainer(id, container);
    ASSERT_TRUE(retval);

    retval = sca->suspendContainer(id);
    ASSERT_TRUE(retval);

    retval = sca->resumeContainer(id);
    ASSERT_TRUE(retval);

    retval = sca->resumeContainer(id);
    ASSERT_FALSE(retval);
}

// Freeze an already frozen container
TEST_F(SoftwareContainerAgentTest, FreezeFrozenContainer) {
    SoftwareContainer *container;
    ContainerID id = sca->createContainer(valid_config);
    bool retval = sca->checkContainer(id, container);
    ASSERT_TRUE(retval);

    retval = sca->suspendContainer(id);
    ASSERT_TRUE(retval);

    retval = sca->suspendContainer(id);
    ASSERT_FALSE(retval);
}

// Freeze an already frozen container, and then resume it
TEST_F(SoftwareContainerAgentTest, DoubleFreezeContainerAndThaw) {
    SoftwareContainer *container;
    ContainerID id = sca->createContainer(valid_config);
    bool retval = sca->checkContainer(id, container);
    ASSERT_TRUE(retval);

    retval = sca->suspendContainer(id);
    ASSERT_TRUE(retval);

    retval = sca->suspendContainer(id);
    ASSERT_FALSE(retval);

    retval = sca->resumeContainer(id);
    ASSERT_TRUE(retval);
}

// Double suspend and then double resume
TEST_F(SoftwareContainerAgentTest, DoubleFreezeAndDoubleThawContainer) {
    SoftwareContainer *container;
    ContainerID id = sca->createContainer(valid_config);
    bool retval = sca->checkContainer(id, container);
    ASSERT_TRUE(retval);

    retval = sca->suspendContainer(id);
    ASSERT_TRUE(retval);

    retval = sca->suspendContainer(id);
    ASSERT_FALSE(retval);

    retval = sca->resumeContainer(id);
    ASSERT_TRUE(retval);

    retval = sca->resumeContainer(id);
    ASSERT_FALSE(retval);
}

// Try to launch a command in a frozen container. Then resume it and try again.
TEST_F(SoftwareContainerAgentTest, FreezeContainerAndLaunchCommand) {
    SoftwareContainer *container;
    ContainerID id = sca->createContainer(valid_config);
    bool retval = sca->checkContainer(id, container);
    ASSERT_TRUE(retval);

    retval = sca->suspendContainer(id);
    ASSERT_TRUE(retval);

    std::function<void (pid_t, int)> noopFunction = [] (pid_t, int) {};

    pid_t pid = sca->launchCommand(id, 0, "whoami", "", "", EnvironmentVariables(), noopFunction);
    ASSERT_EQ(pid, INVALID_PID);

    retval = sca->resumeContainer(id);
    ASSERT_TRUE(retval);

    pid = sca->launchCommand(id, 0, "whoami", "", "", EnvironmentVariables(), noopFunction);
    ASSERT_NE(pid, INVALID_PID);
    waitpid(pid, nullptr, 0);
}

// Make sure you can still shutdown a frozen container
TEST_F(SoftwareContainerAgentTest, ShutdownFrozenContainer) {
    SoftwareContainer *container;
    ContainerID id = sca->createContainer(valid_config);
    bool retval = sca->checkContainer(id, container);
    ASSERT_TRUE(retval);

    retval = sca->suspendContainer(id);
    ASSERT_TRUE(retval);

    retval = sca->shutdownContainer(id);
    ASSERT_TRUE(retval);
}
