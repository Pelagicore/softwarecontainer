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

class SoftwareContainerLibTest : public ::testing::Test
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
    std::unique_ptr<SoftwareContainerLib> lib;
    std::unique_ptr<SoftwareContainerWorkspace> workspace;
};

class SoftwareContainerGatewayTest : public SoftwareContainerLibTest
{
public:
    SoftwareContainerGatewayTest() { }
    ~SoftwareContainerGatewayTest() { }

    void givenContainerIsSet(Gateway *gw);
};
