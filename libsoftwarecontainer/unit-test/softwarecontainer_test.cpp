
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


#include "softwarecontainer_test.h"

void SoftwareContainerGatewayTest::givenContainerIsSet(Gateway *gw)
{
    lib->addGateway(gw);
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

void SoftwareContainerLibTest::SetUp()
{
    ::testing::Test::SetUp();
    workspace = std::make_shared<SoftwareContainerWorkspace>(false);
    lib = std::unique_ptr<SoftwareContainerLib>(new SoftwareContainerLib(workspace));
    lib->setContainerIDPrefix("Test-");
    lib->setMainLoopContext(m_context);
    ASSERT_TRUE(isSuccess(lib->init()));
}

void SoftwareContainerLibTest::TearDown()
{
    ::testing::Test::TearDown();
    lib.reset();
    workspace.reset();
}
