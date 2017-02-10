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

#include "softwarecontaineragent.h"
#include "softwarecontainererror.h"
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
 *
 * This stub is taken from softwarecontaineragent_componenttest
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

/*
class MockSCA :
    public SoftwareContainerAgent
{
public:
    MockSCA(Glib::RefPtr<Glib::MainContext> mainLoopContext,
            std::shared_ptr<Config> config) :
            SoftwareContainerAgent(mainLoopContext, config)
    {

    }

    MOCK_METHOD2(doCreateHelper, void(ContainerID containerID, const SoftwareContainerConfig *config));
    void createHelper(ContainerID containerID, std::unique_ptr<const SoftwareContainerConfig> config) {
        doCreateHelper(containerID, config.get());
    }
};

*/
namespace softwarecontainer {
class TestContainerInterface : public ContainerAgentInterface
{
    LOG_DECLARE_CLASS_CONTEXT("SCAU", "SoftwareContainerAgent Unit Test");
public:
    TestContainerInterface() {};

    ReturnCode startGateways(const GatewayConfiguration &configs __attribute__((unused)))
    {
        return ReturnCode::SUCCESS;
    }

    std::shared_ptr<CommandJob> createCommandJob(const std::string &command)
    {
        log_debug() << "run command : " << command;
        return nullptr;
    }

    void shutdown(unsigned int timeout __attribute__((unused)))
    {
        log_debug() << "Shutting Down.";
    }

    void suspend()
    {
        log_debug() << "Suspending.";
    }

    void resume()
    {
        log_debug() << "Resume.";
    }

    ReturnCode bindMount(const std::string &pathOnHost __attribute__((unused)),
                         const std::string &pathInContainer __attribute__((unused)),
                         bool readonly __attribute__((unused)))
    {
        return ReturnCode::SUCCESS;
    }

    bool previouslyConfigured()
    {
        return true;
    }

};

class TestFactory: public SoftwareContainerFactory
{
public:
    std::shared_ptr<ContainerAgentInterface> createContainer
    (const ContainerID id __attribute__((unused)), std::unique_ptr<const SoftwareContainerConfig> config __attribute__((unused)))
     {
        std::shared_ptr<ContainerAgentInterface> container = std::shared_ptr<ContainerAgentInterface> (new TestContainerInterface());
        return container;
     }
};
} //namespace

class SoftwareContainerAgentTest: public ::testing::Test
{
public:
    SoftwareContainerAgent *sca;

    const std::string configString = "[SoftwareContainer]\n"
#ifdef ENABLE_NETWORKGATEWAY
                                     "create-bridge = true\n"
                                     "bridge-device = " + std::string(BRIDGE_DEVICE_TESTING) + "\n"
                                     "bridge-ip = " + std::string(BRIDGE_IP_TESTING) + "\n"
                                     "bridge-netmask = " + std::string(BRIDGE_NETMASK_TESTING) + "\n"
                                     "bridge-netmask-bitlength = " + std::string(BRIDGE_NETMASK_BITLENGTH_TESTING) +"\n"
                                     "bridge-netaddr = " + std::string(BRIDGE_NETADDR_TESTING) + "\n"
#endif
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

        Glib::RefPtr<Glib::MainContext> mainContext = Glib::MainContext::get_default();
        std::shared_ptr<SoftwareContainerFactory> factory = std::shared_ptr<SoftwareContainerFactory> (new TestFactory());
        sca = new SoftwareContainerAgent(mainContext, config, factory);
    }
    void TearDown() override
    {
        delete sca;
    }
};

/*
 * Test that an agent instance can create, destroy, and create another container.
 */
TEST_F(SoftwareContainerAgentTest, SequenceTest) {
    ASSERT_NO_THROW({
        auto id = sca->createContainer(valid_config);
        sca->suspendContainer(id);
        sca->resumeContainer(id);
        sca->getContainer(id);
        auto containers = sca->listContainers();
        ASSERT_EQ(1, containers.size());

        sca->shutdownContainer(id);
    });
}

TEST_F(SoftwareContainerAgentTest, ExecuteTest) {
    ContainerID id =0;
    ASSERT_NO_THROW(id = sca->createContainer(valid_config));
    ASSERT_THROW({
        EnvironmentVariables var = {};
        sca->execute(id, "run somecommands", "/root", "stdout", var, [&](pid_t, int){});
    }, SoftwareContainerAgentError);
}




