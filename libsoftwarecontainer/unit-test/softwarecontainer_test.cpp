/*
 * Copyright (C) 2016 Pelagicore AB
 * All rights reserved.
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
    workspace = std::unique_ptr<SoftwareContainerWorkspace>(new SoftwareContainerWorkspace());
    lib = std::unique_ptr<SoftwareContainerLib>(new SoftwareContainerLib(*workspace));
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
