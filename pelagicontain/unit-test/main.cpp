/*
 *   Copyright (C) 2014 Pelagicore AB
 *   All rights reserved.
 */
#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "debug.h"

LOG_DEFINE_APP_IDS("PCON", "Pelagicontain");
LOG_DECLARE_CONTEXT(Pelagicontain_DefaultLogContext, "PCON", "Main context");

int main(int argc, char **argv) {
    ConsoleLogOutput logOuput("/dev/null");
    ConsoleLogOutput::setInstance(logOuput);

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
