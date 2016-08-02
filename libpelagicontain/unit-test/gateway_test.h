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
#include "libpelagicontain.h"


class GatewayTest : public ::testing::Test
{
public:

    GatewayTest() { }
    ~GatewayTest() { }

    void SetUp() override;
    void TearDown() override;
    void givenContainerIsSet();

    Glib::RefPtr<Glib::MainContext> m_context = Glib::MainContext::get_default();
    std::unique_ptr<Gateway> gw;
    std::unique_ptr<PelagicontainLib> lib;
    std::unique_ptr<PelagicontainWorkspace> workspace;
};
