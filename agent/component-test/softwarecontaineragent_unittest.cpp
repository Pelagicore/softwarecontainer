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

#include "softwarecontaineragent.h"
#include "config/config.h"
#include "config/configloaderabstractinterface.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace softwarecontainer;

/*
 * Test stub - StringConfigLoader
 *
 * Loads a Glib::KeyFile config from a string, compared to the "real" loader which reads
 * from file.
 */
class StringConfigLoader : public ConfigLoaderAbstractInterface
{

LOG_DECLARE_CLASS_CONTEXT("CFGL", "SoftwareContainer general config loader");

public:
    // Constructor just needs to init parent with the config source string
    StringConfigLoader(const std::string &source) : ConfigLoaderAbstractInterface(source) {}

    std::unique_ptr<Glib::KeyFile> loadConfig() override
    {
        std::unique_ptr<Glib::KeyFile> configData = std::unique_ptr<Glib::KeyFile>(new Glib::KeyFile);
        try {
            configData->load_from_data(Glib::ustring(this->m_source), Glib::KEY_FILE_NONE);
        } catch (Glib::KeyFileError &error) {
            log_error() << "Could not load SoftwareContainer config: \"" << error.what() << "\"";
            throw error;
        }

        return configData;
    }
};


/*
 * Test stub - PreparedConfigDefaults
 *
 * Used for initializing a DefaultConfigs parent with values
 * to support testing.
 */
class PreparedConfigDefaults : public ConfigDefaults
{
public:
    PreparedConfigDefaults(std::map<std::string, std::string> stringOptions,
                           std::map<std::string, int> intOptions,
                           std::map<std::string, bool> boolOptions)
    {
        m_stringOptions = stringOptions;
        m_intOptions = intOptions;
        m_boolOptions = boolOptions;
    }

    ~PreparedConfigDefaults() {}
};


class SoftwareContainerAgentTest: public ::testing::Test
{
public:
    LOG_DECLARE_CLASS_CONTEXT("TEST", "Tester");
    SoftwareContainerAgentTest() { }
    std::shared_ptr<SoftwareContainerAgent> sca;
    Glib::RefPtr<Glib::MainContext> m_context = Glib::MainContext::get_default();
    int m_preloadCount = 0;
    bool m_shutdownContainers = true;
    int m_shutdownTimeout = 1;
    std::shared_ptr<Workspace> workspace;

    // Define minimal required config values
    const std::string configString = "[SoftwareContainer]\n"
                                     "preload-count = 0\n"
                                     "keep-containers-alive = false\n"
                                     "shutdown-timeout = 1\n"
                                     "shared-mounts-dir = " + std::string(SHARED_MOUNTS_DIR_TESTING) + "\n"
                                     "deprecated-lxc-config-path = " + std::string(LXC_CONFIG_PATH_TESTING) + "\n"
                                     "service-manifest-dir = " + std::string(SERVICE_MANIFEST_DIR_TESTING) + "\n"
                                     "default-service-manifest-dir = " + std::string(DEFAULT_SERVICE_MANIFEST_DIR_TESTING);
    const std::string valid_config = "[{\"enableWriteBuffer\": false}]";

    void SetUp() override
    {
        std::unique_ptr<ConfigLoaderAbstractInterface> loader(new StringConfigLoader(configString));

        // Empty defaults, can only be used if the test is never to fall back on default config values
        std::unique_ptr<ConfigDefaults> defaults(
            new PreparedConfigDefaults(std::map<std::string, std::string>(),
                                       std::map<std::string, int>(),
                                       std::map<std::string, bool>()));

        Config config(std::move(loader), std::move(defaults));

        try {
            sca = std::make_shared<SoftwareContainerAgent>(m_context, config);
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
    ContainerID id;
    bool success = sca->createContainer(valid_config, id);
    ASSERT_TRUE(success);
    ASSERT_TRUE(sca->checkContainer(id, container));
}

TEST_F(SoftwareContainerAgentTest, DeleteContainer) {
    SoftwareContainer *container;
    ContainerID id;
    bool success = sca->createContainer(valid_config, id);
    ASSERT_TRUE(success);
    sca->deleteContainer(id);

    ASSERT_FALSE(sca->checkContainer(id, container));
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
    ASSERT_TRUE(sca->parseConfig("[{\"enableWriteBuffer\": true}]"));
    ASSERT_TRUE(workspace->m_enableWriteBuffer);
}

TEST_F(SoftwareContainerAgentTest, parseConfigNice2) {
    ASSERT_TRUE(sca->parseConfig("[{\"enableWriteBuffer\": false}]"));
}

TEST_F(SoftwareContainerAgentTest, parseConfigNoConfig) {
    ASSERT_FALSE(sca->parseConfig(""));
}

TEST_F(SoftwareContainerAgentTest, parseConfigBadConfig) {
    ASSERT_FALSE(sca->parseConfig("gobfmsrfe"));
}

TEST_F(SoftwareContainerAgentTest, parseConfigEvilConfig) {
    // This actually parses and should be true. but wrong parameter name
    ASSERT_TRUE(sca->parseConfig("[{\"enableWritebuffer\": true}]"));
    ASSERT_FALSE(workspace->m_enableWriteBuffer);
}

TEST_F(SoftwareContainerAgentTest, parseConfigEvilConfig2) {
    // This actually parses and should be true
    ASSERT_TRUE(sca->parseConfig("[{\"enableWriteBuffer\": false}]"));
    ASSERT_FALSE(workspace->m_enableWriteBuffer);
}

// Freeze an invalid container
TEST_F(SoftwareContainerAgentTest, FreezeInvalidContainer) {
    ASSERT_FALSE(sca->suspendContainer(0));
}

// Thaw an invalid container
TEST_F(SoftwareContainerAgentTest, ThawInvalidContainer) {
    ASSERT_FALSE(sca->resumeContainer(0));
}

// Thaw a container that has not been frozen
TEST_F(SoftwareContainerAgentTest, ThawUnfrozenContainer) {
    SoftwareContainer *container;
    ContainerID id;
    bool success = sca->createContainer(valid_config, id);
    ASSERT_TRUE(success);

    ASSERT_TRUE(sca->checkContainer(id, container));

    ASSERT_FALSE(sca->resumeContainer(id));
}

// Freeze a container and try to resume it twice
TEST_F(SoftwareContainerAgentTest, FreezeContainerAndThawTwice) {
    SoftwareContainer *container;
    ContainerID id;
    bool success = sca->createContainer(valid_config, id);
    ASSERT_TRUE(success);
    ASSERT_TRUE(sca->checkContainer(id, container));

    ASSERT_TRUE(sca->suspendContainer(id));

    ASSERT_TRUE(sca->resumeContainer(id));

    ASSERT_FALSE(sca->resumeContainer(id));
}

// Freeze an already frozen container
TEST_F(SoftwareContainerAgentTest, FreezeFrozenContainer) {
    SoftwareContainer *container;
    ContainerID id;
    bool success = sca->createContainer(valid_config, id);
    ASSERT_TRUE(success);
    ASSERT_TRUE(sca->checkContainer(id, container));

    ASSERT_TRUE(sca->suspendContainer(id));

    ASSERT_FALSE(sca->suspendContainer(id));
}

// Freeze an already frozen container, and then resume it
TEST_F(SoftwareContainerAgentTest, DoubleFreezeContainerAndThaw) {
    SoftwareContainer *container;
    ContainerID id;
    bool success = sca->createContainer(valid_config, id);
    ASSERT_TRUE(success);
    ASSERT_TRUE(sca->checkContainer(id, container));

    ASSERT_TRUE(sca->suspendContainer(id));

    ASSERT_FALSE(sca->suspendContainer(id));

    ASSERT_TRUE(sca->resumeContainer(id));
}

// Double suspend and then double resume
TEST_F(SoftwareContainerAgentTest, DoubleFreezeAndDoubleThawContainer) {
    SoftwareContainer *container;
    ContainerID id;
    bool success = sca->createContainer(valid_config, id);
    ASSERT_TRUE(success);
    ASSERT_TRUE(sca->checkContainer(id, container));

    ASSERT_TRUE(sca->suspendContainer(id));

    ASSERT_FALSE(sca->suspendContainer(id));

    ASSERT_TRUE(sca->resumeContainer(id));

    ASSERT_FALSE(sca->resumeContainer(id));
}

// Make sure you can still shutdown a frozen container
TEST_F(SoftwareContainerAgentTest, ShutdownFrozenContainer) {
    SoftwareContainer *container;
    ContainerID id;
    bool success = sca->createContainer(valid_config, id);
    ASSERT_TRUE(success);
    ASSERT_TRUE(sca->checkContainer(id, container));

    ASSERT_TRUE(sca->suspendContainer(id));

    ASSERT_TRUE(sca->shutdownContainer(id));
}
