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

#include "softwarecontaineragent.h"
#include "softwarecontainererror.h"
#include "containerconfigparser.h"
#include "config/config.h"
#include "config/configloader.h"
#include "config/mainconfigsource.h"
#include "config/configdefinition.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace softwarecontainer;

/*
 * Test stub - StringConfigLoader
 *
 * Loads a Glib::KeyFile config from a string, compared to the "real" loader which reads
 * from file.
 */
class StringConfigLoader : public ConfigLoader
{

LOG_DECLARE_CLASS_CONTEXT("CFGL", "SoftwareContainer general config loader");

public:
    // Constructor just needs to init parent with the config source string
    StringConfigLoader(const std::string &source) : ConfigLoader(source) {}

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


class SoftwareContainerAgentTest: public ::testing::Test
{
public:

    LOG_DECLARE_CLASS_CONTEXT("TEST", "Tester");

    SoftwareContainerAgentTest() {}

    std::shared_ptr<SoftwareContainerAgent> sca;
    Glib::RefPtr<Glib::MainContext> m_context = Glib::MainContext::get_default();

    // Define minimal required config values
    const std::string configString = "[SoftwareContainer]\n"
#ifdef ENABLE_NETWORKGATEWAY
                                     "create-bridge = true\n"
                                     "bridge-device = " + std::string(BRIDGE_DEVICE_TESTING) + "\n"
                                     "bridge-ip = " + std::string(BRIDGE_IP_TESTING) + "\n"
                                     "bridge-netmask = " + std::string(BRIDGE_NETMASK_TESTING) + "\n"
                                     "bridge-netmask-bits = " + std::string(BRIDGE_NETMASK_BITS_TESTING) +"\n"
                                     "bridge-netaddr = " + std::string(BRIDGE_NETADDR_TESTING) + "\n"
#endif
                                     "keep-containers-alive = false\n"
                                     "shutdown-timeout = 1\n"
                                     "shared-mounts-dir = " + std::string(SHARED_MOUNTS_DIR_TESTING) + "\n"
                                     "deprecated-lxc-config-path = " + std::string(LXC_CONFIG_PATH_TESTING) + "\n"
                                     "service-manifest-dir = " + std::string(SERVICE_MANIFEST_DIR_TESTING) + "\n"
                                     "default-service-manifest-dir = " + std::string(DEFAULT_SERVICE_MANIFEST_DIR_TESTING) + "\n";

    const std::string valid_config = "[{\"enableWriteBuffer\": false}]";

    void SetUp() override
    {
        std::unique_ptr<ConfigLoader> loader(new StringConfigLoader(configString));
        std::unique_ptr<ConfigSource> mainConfig(new MainConfigSource(std::move(loader),
                                                                      ConfigDefinition::typeMap()));

        std::vector<std::unique_ptr<ConfigSource>> configSources;
        configSources.push_back(std::move(mainConfig));

        std::shared_ptr<Config> config = std::make_shared<Config>(std::move(configSources),
                                                                  ConfigDefinition::mandatory(),
                                                                  ConfigDependencies());

        try {
            sca = std::make_shared<SoftwareContainerAgent>(m_context, config);
        } catch(ReturnCode failure) {
            log_error() << "Exception in software agent constructor";
            ASSERT_TRUE(false);
        }
    }

    void TearDown() override
    {
    }
};

/*
 * Test that an agent instance can create, destroy, and create another container.
 */
TEST_F(SoftwareContainerAgentTest, CreateDestroyCreate) {
    ASSERT_NO_THROW({
        ContainerID id1 = sca->createContainer(valid_config);
        sca->deleteContainer(id1);
        ContainerID id2 = sca->createContainer(valid_config);
        sca->deleteContainer(id2);
    });
}

/*
 * Test that an agent instance can create two containers in succession
 */
TEST_F(SoftwareContainerAgentTest, CreateTwice) {
    ASSERT_NO_THROW({
        ContainerID id1 = sca->createContainer(valid_config);
        ContainerID id2 = sca->createContainer(valid_config);
        sca->deleteContainer(id1);
        sca->deleteContainer(id2);
    });
}

TEST_F(SoftwareContainerAgentTest, CreatAndCheckContainer) {
    ASSERT_NO_THROW({
        ContainerID id = sca->createContainer(valid_config);
        sca->getContainer(id);
    });
}

TEST_F(SoftwareContainerAgentTest, DeleteContainer) {
    ContainerID id;
    ASSERT_NO_THROW({
        id = sca->createContainer(valid_config);
        sca->deleteContainer(id);
    });

    ASSERT_THROW(sca->getContainer(id), SoftwareContainerError);
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

// Freeze an invalid container
TEST_F(SoftwareContainerAgentTest, FreezeInvalidContainer) {
    ASSERT_THROW(sca->suspendContainer(0), SoftwareContainerError);
}

// Thaw an invalid container
TEST_F(SoftwareContainerAgentTest, ThawInvalidContainer) {
    ASSERT_THROW(sca->resumeContainer(0), SoftwareContainerError);
}

// Thaw a container that has not been frozen
TEST_F(SoftwareContainerAgentTest, ThawUnfrozenContainer) {
    ASSERT_NO_THROW({
        ContainerID id = sca->createContainer(valid_config);

        sca->getContainer(id);
        sca->resumeContainer(id);
    });
}

// Freeze a container and try to resume it twice
TEST_F(SoftwareContainerAgentTest, FreezeContainerAndThawTwice) {
    ASSERT_NO_THROW({
        ContainerID id = sca->createContainer(valid_config);
        sca->getContainer(id);

        sca->suspendContainer(id);
        sca->resumeContainer(id);
        sca->resumeContainer(id);
    });
}

// Freeze an already frozen container
TEST_F(SoftwareContainerAgentTest, FreezeFrozenContainer) {
    ASSERT_NO_THROW({
        ContainerID id = sca->createContainer(valid_config);
        sca->getContainer(id);

        sca->suspendContainer(id);
        sca->suspendContainer(id);
    });
}

// Freeze an already frozen container, and then resume it
TEST_F(SoftwareContainerAgentTest, DoubleFreezeContainerAndThaw) {
    ASSERT_NO_THROW({
        ContainerID id = sca->createContainer(valid_config);
        sca->getContainer(id);

        sca->suspendContainer(id);
        sca->suspendContainer(id);

        sca->resumeContainer(id);
    });
}

// Double suspend and then double resume
TEST_F(SoftwareContainerAgentTest, DoubleFreezeAndDoubleThawContainer) {
    ASSERT_NO_THROW({
        ContainerID id = sca->createContainer(valid_config);
        sca->getContainer(id);

        sca->suspendContainer(id);
        sca->suspendContainer(id);

        sca->resumeContainer(id);
        sca->resumeContainer(id);
    });
}

// Make sure you can still shutdown a frozen container
TEST_F(SoftwareContainerAgentTest, ShutdownFrozenContainer) {
    ASSERT_NO_THROW({
        ContainerID id = sca->createContainer(valid_config);
        sca->getContainer(id);

        sca->suspendContainer(id);
        sca->shutdownContainer(id);
    });
}

