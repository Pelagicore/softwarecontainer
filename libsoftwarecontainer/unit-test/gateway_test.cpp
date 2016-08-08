/*
 * Copyright (C) 2016 Pelagicore AB
 * All rights reserved.
 */

#include "gateway_test.h"

void GatewayTest::givenContainerIsSet()
{
    lib->getSoftwareContainer().addGateway(*gw);
}

void GatewayTest::SetUp()
{
    ::testing::Test::SetUp();
    workspace = std::unique_ptr<SoftwareContainerWorkspace>(new SoftwareContainerWorkspace());
    lib = std::unique_ptr<SoftwareContainerLib>(new SoftwareContainerLib(*workspace));
    lib->setContainerIDPrefix("Test-");
    lib->setMainLoopContext(m_context);
    ASSERT_TRUE(isSuccess(lib->init()));
    ASSERT_TRUE(gw != nullptr); // Ensure that gw is initialized
}

void GatewayTest::TearDown()
{
    ::testing::Test::TearDown();
    lib.reset();
    gw.reset();
    workspace.reset();
}
