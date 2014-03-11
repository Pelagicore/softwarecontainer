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
 * i.e. there should be a 'rootfs' directory there with a 'controller' executable
 * inside.
 */

namespace {

class PelagicontainComponentTest : public ::testing::Test {

protected:
    std::string m_containerRoot = "/tmp/pc-component-test/";
    std::string m_appRoot = m_containerRoot + "rootfs/";
    std::string m_fifo = m_containerRoot + "rootfs/in_fifo";
    std::string m_outputFile = "/tmp/pc-component-test-output";

    /*! Start the controller in a separate process, wait for controller IPC
     * mechanism to set up the pipe and then create an instance of the
     * ControllerInterface.
     */
    PelagicontainComponentTest()
    {
        // Check that the controller is where we expect it, otherwise we fail now
        std::string controller = m_appRoot + "controller";
        struct stat st;
        if (stat(controller.c_str(), &st) != 0) {
            std::cout << "Error: test did not find controller at: " << controller << std::endl;
            exit(1);
        }

        m_pid = fork();
        if (m_pid == 0) { //Child
            std::string command = controller + " " + m_appRoot;
            system(command.c_str());
            exit(0);
        }

        // The Controller should have opened the fifo before we try to use it
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
        unlink(m_outputFile.c_str());
    }

    ControllerInterface *m_controllerInterface;
    pid_t m_pid;
};

/*! Test that controller executes an 'echo' command.
 *
 * The test calls ControllerInterface::systemCall with a command to echo
 * a string to a file. The file content is used to verify that controller
 * actually called the echo command.
 */
TEST_F(PelagicontainComponentTest, TestControllerExecutesSystemCall) {
    std::string echoedString = "this is my echo test";
    // The '-n' is to not add a newline to the string echoed
    std::string command = "echo -n \"" + echoedString + "\" > " + m_outputFile;
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

    // Read the file content echoed from controller
    std::string fileContent;
    char buf[1024];
    int status = 0;
    for (;;) {
        memset(buf, 0, sizeof(buf));
        status = read(fh, buf, sizeof(buf));
        if (status == -1) {
            perror("read: ");
        } else if (status > 0) {
            fileContent = std::string(buf);
            break;
        }
    }

    int ret = close(fh);
    if (ret == -1) {
        perror("close: ");
    }

    EXPECT_EQ(echoedString, fileContent);
}

/*! Test that controller sets an environment variable.
 *
 * The test calls ControllerInterface::setEnvironmentVariable and then
 * makes controller echo the value of the variable to a file by calling
 * ControllerInterface::systemCall, the content of the file is used to
 * assert the variable value is set as expected.
 */
TEST_F(PelagicontainComponentTest, TestControllerSetsEnvironmentVariable) {
    // Set environment variable
    std::string envVar = "PC_COMP_TEST";
    std::string value = "tested";
    m_controllerInterface->setEnvironmentVariable(envVar, value);

    // Make system call to echo it to file
    std::string echoCommand = "echo -n $" + envVar + " > " + m_outputFile;
    m_controllerInterface->systemCall(echoCommand);

    // Make sure the echo call has been executed so the file is there
    struct stat st;
    while (stat(m_outputFile.c_str(), &st) != 0) {
        ;
    }

    int fh = open(m_outputFile.c_str(), O_RDONLY);
    if (fh == -1) {
        perror("open: ");
    }

    // Read file content and assert it's the same as the variable was set to
    std::string fileContent;
    char buf[1024];
    int status = 0;
    for (;;) {
        memset(buf, 0, sizeof(buf));
        status = read(fh, buf, sizeof(buf));
        if (status == -1) {
            perror("read: ");
        } else if (status > 0) {
            fileContent = std::string(buf);
            break;
        }
    }

    int ret = close(fh);
    if (ret == -1) {
        perror("close: ");
    }

    EXPECT_EQ(value, fileContent);
}

} // namespace
