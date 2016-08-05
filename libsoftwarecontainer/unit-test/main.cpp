/*
 *   Copyright (C) 2014 Pelagicore AB
 *   All rights reserved.
 */
#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "ivi-logging-console.h"
#include "softwarecontainer-common.h"

LOG_DEFINE_APP_IDS("PCON", "SoftwareContainer Unit Test");
LOG_DECLARE_CONTEXT(SoftwareContainer_DefaultLogContext, "PCON", "Main context");

int main(int argc, char * *argv)
{
    if (!std::getenv("LOG_OUTPUT")) {
        // Silence the logger
        logging::ConsoleLogContext::setGlobalLogLevel(logging::LogLevel::None);
    }

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
