/*
 * Copyright (C) 2016 Pelagicore AB
 * All rights reserved.
 */

#include <thread>
#include <sys/un.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "gateway.h"
#include "generators.h"
#include "libsoftwarecontainer.h"


class GatewayTest : public ::testing::Test
{
public:

    GatewayTest() { }
    ~GatewayTest() { }

    void SetUp() override;
    void TearDown() override;
    void givenContainerIsSet(Gateway *gw);

    Glib::RefPtr<Glib::MainContext> m_context = Glib::MainContext::get_default();
    std::unique_ptr<SoftwareContainerLib> lib;
    std::unique_ptr<SoftwareContainerWorkspace> workspace;
};
