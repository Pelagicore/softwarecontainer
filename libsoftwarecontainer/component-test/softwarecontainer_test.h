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

/*
#include <thread>
#include <stdlib.h>
*/

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "gateway/gateway.h"
#include "softwarecontainer.h"
#include "config/softwarecontainerconfig.h"

using namespace softwarecontainer;

/*
 * This is the base class for both the LibTest and GatewayTest.
 *
 * It contains some utilities and some things that are common
 * for both test classes.
 */
class SoftwareContainerComponentTest : public ::testing::Test
{
public:
    SoftwareContainerComponentTest() { }
    ~SoftwareContainerComponentTest() { }

    std::unique_ptr<SoftwareContainerConfig> createConfig();
    std::string getTempPath(bool directory);
    std::string createTempFile(const std::string &prefix = "");
    std::string createTempDir(const std::string &prefix = "");

    void TearDown() override;

private:
    std::vector<std::string> filesToRemove;
    std::vector<std::string> dirsToRemove;
};

/*
 * Test class to be used for testing the full library
 */
class SoftwareContainerLibTest : public SoftwareContainerComponentTest
{
public:
    SoftwareContainerLibTest() { }
    ~SoftwareContainerLibTest() { }

    void SetUp() override;
    void TearDown() override;
    void run();
    void exit();

    Glib::RefPtr<Glib::MainContext> m_context = Glib::MainContext::get_default();
    Glib::RefPtr<Glib::MainLoop> m_ml;
    std::unique_ptr<SoftwareContainer> m_sc;
};

/*
 * Test class to be used for testing the gateways
 */
class SoftwareContainerGatewayTest : public SoftwareContainerComponentTest
{
public:
    SoftwareContainerGatewayTest() { }
    ~SoftwareContainerGatewayTest() { }

    // Gateway config
    json_t *jsonConfig = nullptr;
    void loadConfig(const std::string &config);

    // Container
    std::shared_ptr<softwarecontainer::ContainerAbstractInterface> m_container;
    pid_t m_initPid;

    // Test setup / teardown
    void TearDown() override;
    void SetUp() override;

};
