/*
 *   Copyright (C) 2014 Pelagicore AB
 *   All rights reserved.
 */
#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include <fcntl.h>

#include "fifoipc.h"
#include "abstractcontroller.h"

class MockAbstractController :
    public AbstractController
{
public:
    MOCK_METHOD0(runApp, int());
    MOCK_METHOD0(killApp, void());
    MOCK_METHOD2(setEnvironmentVariable, void(const std::string &, const std::string &));
    MOCK_METHOD1(systemCall, void(const std::string &));
};

using ::testing::InSequence;

TEST(FifoIPCTest, TestShouldCallRunAppAndKillApp) {
//     MockAbstractController controller;
//     FifoIPC fifoIPC(&controller);

    std::string fifoPath("/tmp/in_fifo");

    pid_t pid = fork();
    if (pid == 0) { // Child
        MockAbstractController controller;
        FifoIPC fifoIPC(&controller);
        fifoIPC.initialize(fifoPath);
        // The calls should be made in the specific order as below:
        {
            InSequence sequence;
            EXPECT_CALL(controller, killApp()).Times(1);
    //         EXPECT_CALL(pam, updateFinished(appId)).Times(1);
    //         EXPECT_CALL(pam, unregisterClient(appId)).Times(1);
        }

        exit(0);
    } else if (pid == -1) {
        perror("fork: ");
    }

    // Make sure FifoIPC has had time to create the pipe
    struct stat st;
    while (stat(fifoPath.c_str(), &st) != 0) {
        ;
    }

    int fifo = open(fifoPath.c_str(), O_WRONLY);
    if (fifo == -1)
        std::cout << "open: " << strerror(errno) << std::endl;

    char msg[] = {'2', '\0'}; //Shutdown
    write(fifo, msg, sizeof(msg));

    int status;
    wait(&status);

    // FifoIPC can't remove the pipe here as it is initialized in the fork,
    // so we do it manually to clean up. A warning from the unlink call
    // from the instance in the fork can be expected in the test output due to this.
    unlink(fifoPath.c_str());
}
