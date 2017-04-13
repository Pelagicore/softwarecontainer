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

#include "softwarecontainer_test.h"
#include "container.h"

void SoftwareContainerGatewayTest::SetUp()
{
    SoftwareContainerComponentTest::SetUp();

    // Create a container from a test config
    auto config = createConfig();
    m_container = std::shared_ptr<ContainerAbstractInterface>(
        new Container("SC-0",
                      config->containerConfigPath(),
                      config->sharedMountsDir(),
                      config->writeBufferEnabled(),
                      config->containerShutdownTimeout()
        )
    );

    ASSERT_TRUE(m_container->initialize());
    ASSERT_TRUE(m_container->create());
    ASSERT_TRUE(m_container->start(&m_initPid));
}

void SoftwareContainerGatewayTest::TearDown()
{
    SoftwareContainerComponentTest::TearDown();

    if (nullptr != jsonConfig) {
        json_decref(jsonConfig);
    }

    m_container->shutdown();
}

void SoftwareContainerGatewayTest::loadConfig(const std::string &config)
{
    if (nullptr != jsonConfig) {
        json_decref(jsonConfig);
    }

    json_error_t err;
    jsonConfig = json_loads(config.c_str(), 0, &err);
}

void SoftwareContainerLibTest::SetUp()
{
    SoftwareContainerComponentTest::SetUp();

    // Create a config
    std::unique_ptr<SoftwareContainerConfig> config = createConfig();

    // Create random container id
    srand(time(NULL));
    uint32_t containerId =  rand() % 100;

    // Create SC lib instance
    m_sc = std::unique_ptr<SoftwareContainer>(new SoftwareContainer(containerId, std::move(config)));
}

void SoftwareContainerLibTest::TearDown()
{
    m_sc.reset();
    SoftwareContainerComponentTest::TearDown();
}

void SoftwareContainerLibTest::run()
{
    m_ml = Glib::MainLoop::create(m_context);
    m_ml->run();
}

void SoftwareContainerLibTest::exit()
{
    m_ml->quit();
}

void SoftwareContainerComponentTest::TearDown()
{
    ::testing::Test::TearDown();

    for (auto &file: filesToRemove) {
        unlink(file.c_str());
    }
    filesToRemove.clear();

    for (auto &dir: dirsToRemove) {
        rmdir(dir.c_str());
    }
    dirsToRemove.clear();
}

/*
 * Set up the workspace with all the config values it needs.
 */
std::unique_ptr<SoftwareContainerConfig> SoftwareContainerComponentTest::createConfig()
{
    std::unique_ptr<SoftwareContainerConfig> config =
        std::unique_ptr<SoftwareContainerConfig>(new SoftwareContainerConfig(
#ifdef ENABLE_NETWORKGATEWAY
        // Setup network stuff
        true, // createBridge
        std::string(BRIDGE_DEVICE_TESTING),// Should be set be CMake
        std::string(BRIDGE_IP_TESTING),// Should be set be CMake
        std::string(BRIDGE_NETMASK_TESTING),// Should be set be CMake
        std::stoi(BRIDGE_NETMASK_BITLENGTH_TESTING),// Should be set be CMake
        std::string(BRIDGE_NETADDR_TESTING),// Should be set be CMake
#endif // ENABLE_NETWORKGATEWAY
        LXC_CONFIG_PATH_TESTING, // Should be set be CMake
        SHARED_MOUNTS_DIR_TESTING, // Should be set be CMake
        1 // timeout
    ));

    config->setEnableWriteBuffer(false);
    return config;
}

std::string SoftwareContainerComponentTest::createTempFile(const std::string &prefix)
{
    std::string strWithPrefix = "SC-tmpFileXXXXXX";
    if (prefix.empty()) {
        strWithPrefix = buildPath("/tmp", strWithPrefix);
    } else {
        strWithPrefix = buildPath(prefix, strWithPrefix);
    }

    size_t n = strWithPrefix.size() + 1;
    char *fileTemplate = new char[n];
    strncpy(fileTemplate, strWithPrefix.c_str(), n);

    int fd = mkstemp(fileTemplate);
    close(fd);

    std::string filename(fileTemplate);
    delete[] fileTemplate;

    filesToRemove.push_back(filename);
    return filename;
}

std::string SoftwareContainerComponentTest::createTempDir(const std::string &prefix)
{
    std::string strWithPrefix = "SC-tmpDirXXXXXX";
    if (prefix.empty()) {
        strWithPrefix = buildPath("/tmp", strWithPrefix);
    } else {
        strWithPrefix = buildPath(prefix, strWithPrefix);
    }

    size_t n = strWithPrefix.size() + 1;
    char *dirTemplate = new char[n];
    strncpy(dirTemplate, strWithPrefix.c_str(), n);

    mkdtemp(dirTemplate);

    std::string dirname(dirTemplate);
    delete[] dirTemplate;

    dirsToRemove.push_back(dirname);
    return dirname;
}

/*
 * Create a temporary directory or file, and optionally remove it.
 * Removal is useful if one only wants a unique tmp name
 */
std::string SoftwareContainerComponentTest::getTempPath(bool directory)
{
    std::string name;

    if (directory) {
        name = createTempDir();
        dirsToRemove.pop_back();
        rmdir(name.c_str());

    } else {
        name = createTempFile();
        unlink(name.c_str());
        filesToRemove.pop_back();
    }

    return name;
}
