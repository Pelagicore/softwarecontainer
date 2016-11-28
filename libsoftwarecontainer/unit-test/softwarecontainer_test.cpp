
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
    sc->addGateway(gw);
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

void SoftwareContainerTest::SetUp()
{
    ::testing::Test::SetUp();
    ASSERT_NO_THROW({
        workspace = std::make_shared<Workspace>(false);
    });
    srand(time(NULL));
    uint32_t containerId =  rand() % 100;
    sc = std::unique_ptr<SoftwareContainer>(new SoftwareContainer(workspace, containerId));
    sc->setMainLoopContext(m_context);
    ASSERT_TRUE(isSuccess(sc->init()));
}

void SoftwareContainerTest::TearDown()
{
    ::testing::Test::TearDown();
    sc.reset();
    workspace.reset();
}
