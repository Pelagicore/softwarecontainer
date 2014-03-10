/*
 *   Copyright (C) 2014 Pelagicore AB
 *   All rights reserved.
 */
#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include <fcntl.h>

#include "controllerinterface.h"

/*! This module contains tests for Pelagicontain which are not unit tests
 * but also not proper component tests.
 *
 * The entry point is basically the same as the main method would have, so
 * some classes can be stubbed or mocked while still allowing to run the
 * actual Controller for example.
 *
 * These tests assumes there is a container root set up in '/tmp/pc-component-test/'
 * i.e. there should be a 'rootfs' directory there with a 'containedapp' executable
 * as well as the 'controller' executable there. The network bridge needs to be
 * set up as well.
 */

namespace {

class PelagicontainComponentTest : public ::testing::Test {

protected:
    /*! Start the controller in a separate process, wait for controller IPC
     * mechanism to set up the pipe and then create an instance of the
     * ControllerInterface.
     */
    PelagicontainComponentTest()
    {
        m_pid = fork();
        if (m_pid == 0) { //Child
            std::string controllerCommand = m_appRoot + "controller";
            std::string command = controllerCommand + " " + m_appRoot;

            system(command.c_str());

            exit(0);
        }

        // The Controller should have opened the fifo before we try to use it
        struct stat st;
        while (stat(m_fifo.c_str(), &st) != 0) {
            ;
        }

        m_controllerInterface = new ControllerInterface(m_containerRoot);
    }

    /*! Shutdown the controller and remove test output file.
     */
    virtual ~PelagicontainComponentTest()
    {
        m_controllerInterface->shutdown();
        int status;
        waitpid(m_pid, &status, 0);
        delete m_controllerInterface;
        unlink("/tmp/pc-component-test-output");
    }
    
    ControllerInterface *m_controllerInterface;
    pid_t m_pid;
    
    std::string m_containerRoot = "/tmp/pc-component-test/";
    std::string m_appRoot = m_containerRoot + "rootfs/";
    std::string m_fifo = m_containerRoot + "rootfs/in_fifo";
    std::string m_outputFile = "/tmp/pc-component-test-output";
};

/*! Test that controller executes an 'echo' command inside the container.
 *
 * The test calls ControllerInterface::systemCall with a command to echo
 * a string to a file. The file content is used to verify that controller
 * actually called the echo command.
 */
TEST_F(PelagicontainComponentTest, TestControllerExecutesSystemCall) {
    std::string theString = "this is my echo test";
    // The '-n' is to not add a newline to the string echoed
    std::string command = "echo -n \"" + theString + "\" > " + m_outputFile;
    m_controllerInterface->systemCall(command);

    // Make sure the call has been executed
    struct stat st;
    while (stat(m_outputFile.c_str(), &st) != 0) {
        ;
    }

    int fh = open(m_outputFile.c_str(), O_RDONLY);
    if (fh == -1) {
        perror("open: ");
    }

    // Read the file content echoed from inside the contaier
    std::string content;
    char buf[1024];
    int status = 0;
    for (;;) {
        memset(buf, 0, sizeof(buf));
        status = read(fh, buf, sizeof(buf));
        if (status > 0) {
            content = std::string(buf);
            break;
        }
    }
    if (status == -1) {
        perror("read: ");
    }

    int ret = close(fh);
    if (ret == -1) {
        perror("close: ");
    }

    EXPECT_EQ(std::string(theString), content);
}

} // namespace
