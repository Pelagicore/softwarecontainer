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


void SoftwareContainerGatewayTest::givenContainerIsSet(Gateway *gw)
{
    m_sc->addGateway(gw);
}

void SoftwareContainerTest::run()
{
    m_ml = Glib::MainLoop::create(m_context);
    m_ml->run();
}

void SoftwareContainerTest::exit()
{
    m_ml->quit();
}

/* 
 * Set up the workspace with all the config values it needs.
 */ 
std::unique_ptr<SoftwareContainerConfig> SoftwareContainerTest::createConfig()
{
    std::unique_ptr<SoftwareContainerConfig> config = 
        std::unique_ptr<SoftwareContainerConfig>(new SoftwareContainerConfig(
#ifdef ENABLE_NETWORKGATEWAY
        // Setup network stuff
        true, // createBridge
        std::string(BRIDGE_DEVICE_TESTING),// Should be set be CMake
        std::string(BRIDGE_IP_TESTING),// Should be set be CMake
        std::string(BRIDGE_NETMASK_TESTING),// Should be set be CMake
        std::stoi(BRIDGE_NETMASK_BITS_TESTING),// Should be set be CMake
        std::string(BRIDGE_NETADDR_TESTING),// Should be set be CMake
#endif // ENABLE_NETWORKGATEWAY
        LXC_CONFIG_PATH_TESTING, // Should be set be CMake
        SHARED_MOUNTS_DIR_TESTING, // Should be set be CMake
        1 // timeout
    ));
    
    config->setEnableWriteBuffer(false);
    return config;
}

void SoftwareContainerTest::SetUp()
{
    ::testing::Test::SetUp();

    std::unique_ptr<SoftwareContainerConfig> config = createConfig();

    srand(time(NULL));
    uint32_t containerId =  rand() % 100;

    m_sc = std::unique_ptr<SoftwareContainer>(new SoftwareContainer(containerId, std::move(config)));
}

void SoftwareContainerTest::TearDown()
{
    ::testing::Test::TearDown();
    m_sc.reset();
}

/*
 * Create a temporary directory or file, and optionally remove it.
 * Removal is useful if one only wants a unique tmp name
 */
std::string SoftwareContainerTest::getTempPath(bool directory, bool shouldUnlink)
{
    char fileTemplate[] = "/tmp/SC-tmpXXXXXX";
    int fd = 0;

    if (directory) {
        mkdtemp(fileTemplate);
        if (shouldUnlink) {
            rmdir(fileTemplate);
        }
    } else {
        fd = mkstemp(fileTemplate);
        close(fd);
        if (shouldUnlink) {
            unlink(fileTemplate);
        }
    }

    return std::string(fileTemplate);
}
