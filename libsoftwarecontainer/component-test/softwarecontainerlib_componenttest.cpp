/*
 * Copyright (C) 2016-2017 Pelagicore AB
 *
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
 * WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR
 * BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES
 * OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
 * ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 *
 * For further information see LICENSE
 */


#include "softwarecontainer_test.h"

#include "gateway/pulsegateway.h"
#include "gateway/waylandgateway.h"
#include "gateway/dbus/dbusgateway.h"
#include "gateway/network/networkgateway.h"
#include "gateway/files/filegateway.h"

#include "functionjob.h"
#include "commandjob.h"


LOG_DECLARE_DEFAULT_CONTEXT(defaultContext, "ff", "dd");

class SoftwareContainerApp : public SoftwareContainerTest
{

public:

    void startGateways(const std::string &config, const std::string &gatewayID)
    {
        json_error_t error;
        json_t *configJson = json_loads(config.c_str(), 0, &error);
        ASSERT_FALSE(configJson == nullptr);

        GatewayConfiguration gwConf;
        gwConf.append(gatewayID, configJson);

        getSc().startGateways(gwConf);

        json_decref(configJson);
    }

    bool bindMountInContainer(const std::string src, const std::string dst, bool readOnly)
    {
        return getSc().bindMount(src, dst, readOnly);
    }

    Glib::RefPtr<Glib::MainContext> getMainContext()
    {
        return m_context;
    }

    SoftwareContainer &getSc()
    {
        return *m_sc;
    }

};

TEST_F(SoftwareContainerApp, TestWaylandWhitelist) {

    GatewayConfiguration config;
    std::string configStr = "[ { \"enabled\" : true },\
                               { \"enabled\" : false } ]";
    startGateways(configStr, WaylandGateway::ID);

    auto jobTrue = getSc().createFunctionJob([] (){
        bool ERROR = 1;
        bool SUCCESS = 0;

        bool hasWayland = false;
        std::string waylandDir = Glib::getenv(WaylandGateway::WAYLAND_RUNTIME_DIR_VARIABLE_NAME,
                                              hasWayland);
        if (!hasWayland) {
            log_error() << "No wayland dir";
            return ERROR;
        }
        log_debug() << "Wayland dir : " << waylandDir;
        std::string socketPath = buildPath(waylandDir, WaylandGateway::SOCKET_FILE_NAME);
        log_debug() << "isSocket : " << socketPath << " " << isSocket(socketPath);
        if ( !isSocket(socketPath) ) {
            return ERROR;
        }

        return SUCCESS;

    });
    jobTrue->start();
    jobTrue->wait();
    ASSERT_TRUE(jobTrue->isSuccess());

    auto westonJob = getSc().createCommandJob("/bin/sh -c \"/usr/bin/weston-info > /dev/null\"");
    westonJob->start();
    westonJob->wait();
    ASSERT_TRUE(westonJob->isSuccess());

}

/*
 * Test that it is not possible to create two containers with the same id.
 */
TEST_F(SoftwareContainerApp, DoubleIDCreatesError) {
    const ContainerID id = 1;

    // Containers need to be created in the same scope since they will
    // shut down and "release" their ID when the object is destroyed
    // when it goes out of scope.
    ASSERT_THROW({
        SoftwareContainer s1(id, createConfig());
        SoftwareContainer s2(id, createConfig());
    }, SoftwareContainerError);
}

TEST_F(SoftwareContainerApp, EnvVarsSet) {
    auto job = getSc().createFunctionJob([&] () {
        bool hasTestVar = false;
        Glib::getenv("TESTVAR", hasTestVar);
        return hasTestVar ? 0 : 1;
    });
    job->setEnvironmentVariable("TESTVAR","YES");
    job->start();
    job->wait();
    ASSERT_TRUE(job->isSuccess());
}

static constexpr int EXISTENT = 1;
static constexpr int NON_EXISTENT = 0;

TEST_F(SoftwareContainerApp, FileGatewayReadOnly) {

    std::string tempFilename1 = createTempFile();

    // They will be mapped to these files, which should not yet exist
    std::string containerPath1 = "/tmp/testFile1";
    auto jobMounted = getSc().createFunctionJob([&] () {
        return isFile(containerPath1) ? EXISTENT : NON_EXISTENT;
    });
    jobMounted->start();
    ASSERT_EQ(jobMounted->wait(), NON_EXISTENT);

    // Let's configure the env gateway
    std::string configStr =
    "["
        "{"
            "  \"path-host\" : \"" + tempFilename1 + "\""
            ", \"path-container\" : \"" + containerPath1 + "\""
            ", \"read-only\": true"
        "}"
    "]";
    startGateways(configStr, FileGateway::ID);

    jobMounted->start();
    ASSERT_EQ(jobMounted->wait(), EXISTENT);

    // Write some data to the files outside the container and make sure we can
    // read it inside the container
    std::string testData = "testdata";
    writeToFile(tempFilename1, testData);

    // Test if we can read the data back into a variable
    auto jobReadData = getSc().createFunctionJob([&] () {
        std::string readBack;
        readFromFile(containerPath1, readBack);
        return readBack == testData ? 0 : 1;
    });
    jobReadData->start();
    jobReadData->wait();
    ASSERT_TRUE(jobReadData->isSuccess());

    // Make sure we can't write to the file
    std::string badData = "This data should never be read";
    auto jobWriteDataRO = getSc().createFunctionJob([&] () {
        writeToFile(containerPath1, badData);
        return 0;
    });
    jobWriteDataRO->start();
    jobWriteDataRO->wait();

    std::string readBack = "";
    readFromFile(tempFilename1, readBack);
    ASSERT_EQ(readBack, testData);
}

TEST_F(SoftwareContainerApp, FileGatewayReadWrite) {

    std::string tempFilename1 = createTempFile();

    // They will be mapped to these files
    std::string containerPath1 = "/tmp/testFile1";
    auto jobMounted = getSc().createFunctionJob([&] () {
        return isFile(containerPath1) ? EXISTENT : NON_EXISTENT;
    });
    jobMounted->start();
    ASSERT_EQ(jobMounted->wait(), NON_EXISTENT);

    std::string configStr =
    "["
        "{" // The files below are mounted read-only
            "  \"path-host\" : \"" + std::string(tempFilename1) + "\""
            ", \"path-container\" : \"" + containerPath1 + "\""
            ", \"read-only\": false"
        "}"
    "]";
    startGateways(configStr, FileGateway::ID);

    jobMounted->start();
    ASSERT_EQ(jobMounted->wait(), EXISTENT);

    // Write some data to the files outside the container and make sure we can
    // read it inside the container
    std::string testData = "testdata";
    writeToFile(tempFilename1, testData);

    // Test if we can read the data back into a variable
    auto jobReadData = getSc().createFunctionJob([&] () {
        std::string readBack;
        readFromFile(containerPath1, readBack);
        return readBack == testData ? 0 : 1;
    });
    jobReadData->start();
    jobReadData->wait();
    ASSERT_TRUE(jobReadData->isSuccess());

    // Make sure we can write to the file
    std::string newData = "This data should have been written";
    auto jobWriteData = getSc().createFunctionJob([&] () {
        writeToFile(containerPath1, newData);
        return 0;
    });
    jobWriteData->start();
    jobWriteData->wait();

    std::string readBack = "";
    readFromFile(tempFilename1, readBack);
    ASSERT_EQ(newData, readBack);
}

/**
 * Test whether the mounting of files works properly
 */
TEST_F(SoftwareContainerApp, TestFileMounting) {

    std::string tempFilename = createTempFile();

    std::string content = "GFDGDFHDHRWG";
    writeToFile(tempFilename, content);

    auto job = getSc().createFunctionJob([&] () {
        return isFile(tempFilename) ? EXISTENT : NON_EXISTENT;
    });
    job->start();

    // File should not be available
    ASSERT_EQ(job->wait(), NON_EXISTENT);

    ASSERT_TRUE(bindMountInContainer(tempFilename, tempFilename, true));

    // Now file should be available
    job->start();
    ASSERT_EQ(job->wait(), EXISTENT);
}

/**
 * Test that it is not possible to mount to same place twice
 */
TEST_F(SoftwareContainerApp, TestDoubleMounting) {

    std::string tempFilename = createTempFile();

    // create a temporary file with some content
    std::string content = "GFDGDFHDHRWG";
    writeToFile(tempFilename, content);

    // Make sure that the file is not already in the container
    auto job = getSc().createFunctionJob([&] () {
        return isFile(tempFilename) ? EXISTENT : NON_EXISTENT;
    });
    job->start();
    ASSERT_EQ(job->wait(), NON_EXISTENT);

    // Bind mount the file
    ASSERT_TRUE(bindMountInContainer(tempFilename, tempFilename, true));

    // Check that the file is now in the container
    job->start();
    ASSERT_EQ(job->wait(), EXISTENT);

    // Try to bind mount again. This should fail!
    ASSERT_FALSE(bindMountInContainer(tempFilename, tempFilename, true));

    // Check that the file is still in the container
    job->start();
    ASSERT_EQ(job->wait(), EXISTENT);
}

/**
 * Test whether it should not be possible to mount over LXC mounts
 */
TEST_F(SoftwareContainerApp, TestMountingOverRoot) {

    std::string tempDirname = createTempDir();
    ASSERT_TRUE(isDirectory(tempDirname));

    ASSERT_FALSE(bindMountInContainer(tempDirname, "/", false));
    ASSERT_FALSE(bindMountInContainer(tempDirname, "/lib", false));
}

/**
 * Test whether the mounting of folder works properly
 */
TEST_F(SoftwareContainerApp, TestFolderMounting) {

    std::string tempDirname = createTempDir();

    auto job1 = getSc().createFunctionJob([&] () {
        return isDirectory(tempDirname) ? EXISTENT : NON_EXISTENT;
    });
    job1->start();
    ASSERT_EQ(job1->wait(), NON_EXISTENT);

    ASSERT_TRUE(bindMountInContainer(tempDirname, tempDirname, false));

    job1->start();
    ASSERT_EQ(job1->wait(), EXISTENT);

    // Test that an unexisting path is created when you mount to it
    std::string longUnexistingPath = "/var/apa/bepa/cepa";
    ASSERT_TRUE(bindMountInContainer(tempDirname, longUnexistingPath, false));
    auto job2 = getSc().createFunctionJob([&] () {
        return isDirectory(longUnexistingPath) ? EXISTENT : NON_EXISTENT;
    });
    job2->start();
    ASSERT_EQ(job2->wait(), EXISTENT);

    // Write some data to a file inside the directory
    std::string tempFilename = createTempFile(tempDirname);
    std::string content = "GFDGDFHDHRWG";

    writeToFile(tempFilename, content);

    auto job3 = getSc().createFunctionJob([&] () {
        return isFile(std::string(tempFilename)) ? EXISTENT : NON_EXISTENT;
    });
    job3->start();
    ASSERT_EQ(job3->wait(), EXISTENT);

}

#include <stdlib.h>

/**
 * Test whether the mounting of sockets works properly
 */
TEST_F(SoftwareContainerApp, TestUnixSocket) {

    std::string tempDirname = createTempDir();
    ASSERT_TRUE(bindMountInContainer(tempDirname, tempDirname, false));

    std::string strUnixSocket = buildPath(tempDirname, "socket");
    const char *tempUnixSocket = strUnixSocket.c_str();

    auto job1 = getSc().createFunctionJob([&] () {
                int fd, fd2, done, n;
                char str[100];
                socklen_t t;
                sockaddr_un local, remote;
                local.sun_family = AF_UNIX;
                strncpy(local.sun_path, tempUnixSocket, strlen(tempUnixSocket) + 1);
                fd = socket(AF_UNIX, SOCK_STREAM, 0);
                bind(fd, (sockaddr*)(&local), sizeof(local));
                listen(fd, 100);

                t = sizeof(remote);
                if ((fd2 = accept(fd, (struct sockaddr *)&remote, &t)) == -1) {
                    return 0;
                }

                done = 0;
                do {
                    n = recv(fd2, str, 100, 0);
                    if (n <= 0) {
                        if (n < 0) perror("recv");
                        done = 1;
                    }

                } while (!done);

                close(fd2);

                return done == 1 ? EXISTENT : NON_EXISTENT;
            });


    pid_t forkpid = fork();

    if(forkpid < 0)
    {
        printf("\n Fork failed, quitting!!!!!!\n");
        return;
    }

    if(forkpid == 0) // child process
    {
        int s, len;
        struct sockaddr_un remote;
        char str[] = "TestData";

        sleep(1);

        if ((s = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
            _exit(NON_EXISTENT);
        }

        remote.sun_family = AF_UNIX;
        strncpy(remote.sun_path, tempUnixSocket, strlen(tempUnixSocket) + 1);
        len = strlen(remote.sun_path) + sizeof(remote.sun_family);
        if (connect(s, (struct sockaddr *)&remote, len) == -1) {
            _exit(NON_EXISTENT);
        }


        if (send(s, str, strlen(str), 0) == -1) {
            _exit(NON_EXISTENT);
        }

        close(s);
        _exit(EXISTENT);
    }
    else //Parent process
    {
        job1->start();
        ASSERT_EQ(job1->wait(), EXISTENT);
    }
}


TEST_F(SoftwareContainerApp, DISABLED_TestPulseAudioEnabled) {

    std::string configStr = "[ { \"audio\" : true } ]";
    startGateways(configStr, PulseGateway::ID);

    // We need access to the test file, so we bind mount it
    std::string soundFile = buildPath(TEST_DATA_DIR, std::string("Rear_Center.wav"));
    ASSERT_TRUE(bindMountInContainer(soundFile, soundFile, true));

    // Make sure the file is there
    auto job1 = getSc().createFunctionJob([&] () {
        return isFile(soundFile) ? EXISTENT : NON_EXISTENT;
    });
    job1->start();
    ASSERT_EQ(job1->wait(), EXISTENT);

    auto job2 = getSc().createCommandJob("/usr/bin/paplay " + soundFile);
    job2->start();
    ASSERT_TRUE(job2->isRunning());
    job2->wait();
    ASSERT_TRUE(job2->isSuccess());
}


TEST_F(SoftwareContainerApp, TestStdin) {
    auto job = getSc().createCommandJob("/bin/cat");
    job->captureStdin();
    job->captureStdout();
    job->start();
    ASSERT_TRUE(job->isRunning());

    const char outputBytes[] = "test string";
    char inputBytes[sizeof(outputBytes)] = {};

    unsigned int writtenBytesCount = write(job->stdin(), outputBytes, sizeof(outputBytes));
    ASSERT_EQ(writtenBytesCount, sizeof(outputBytes));

    unsigned int readBytesCount = read(job->stdout(), inputBytes, sizeof(inputBytes));
    ASSERT_EQ(readBytesCount, sizeof(outputBytes));

    SignalConnectionsHandler connections;
    addProcessListener(connections, job->pid(), [&] (
                int pid, int exitCode) {
                log_debug() << "Finished process:" << job->toString();
                log_debug() << "Pid was " << pid << ", exitCode was " << exitCode;
                exit();
            }, getMainContext());

    kill(job->pid(), SIGTERM);

    run();
}


#ifdef ENABLE_NETWORKGATEWAY

/**
 * We do not enable the network gateway so we expect the ping to fail
 */
TEST_F(SoftwareContainerApp, TestNetworkInternetCapabilityDisabled) {
    auto job = getSc().createCommandJob("/bin/sh -c \"ping www.google.com -c 5 -q > /dev/null 2>&1\"");
    job->start();
    job->wait();
    ASSERT_TRUE(job->isError());

    auto job2 = getSc().createCommandJob("/bin/sh -c \"ping 8.8.8.8 -c 5 -q > /dev/null 2>&1\"");
    job2->start();
    job2->wait();
    ASSERT_TRUE(job2->isError());
}

/**
 * We can also disable it explicitly
 */
TEST_F(SoftwareContainerApp, TestNetworkInternetCapabilityDisabledExplicit) {
    std::string configStr =
        "[{"
            "\"direction\": \"OUTGOING\","
            "\"allow\": []"
        "}]";
    startGateways(configStr, NetworkGateway::ID);

    auto job = getSc().createCommandJob("/bin/sh -c \"ping www.google.com -c 5 -q > /dev/null 2>&1\"");
    job->start();
    job->wait();
    ASSERT_TRUE(job->isError());

    auto job2 = getSc().createCommandJob("/bin/sh -c \"ping 8.8.8.8 -c 5 -q > /dev/null 2>&1\"");
    job2->start();
    job2->wait();
    ASSERT_TRUE(job2->isError());
}

/**
 * This test checks that an external is accessible after the network gateway has been enabled to access the internet.
 */
TEST_F(SoftwareContainerApp, TestNetworkInternetCapabilityEnabled) {
    std::string configStr =
        "[{"
            "\"direction\": \"OUTGOING\","
            "\"allow\": ["
                         "{ \"host\": \"*\", \"protocols\": \"icmp\"},"
                         "{ \"host\": \"*\", \"protocols\": [\"udp\", \"tcp\"], \"ports\": 53}"
                       "]"
        "},"
        "{"
            "\"direction\": \"INCOMING\","
            "\"allow\": ["
                         "{ \"host\": \"*\", \"protocols\": \"icmp\"},"
                         "{ \"host\": \"*\", \"protocols\": [\"udp\", \"tcp\"], \"ports\": 53}"
                       "]"
        "}]";
    startGateways(configStr, NetworkGateway::ID);

    auto job = getSc().createCommandJob("/bin/sh -c \"ping example.com -c 5 -q > /dev/null\"");
    job->start();
    job->wait();
    ASSERT_TRUE(job->isSuccess());
}

#endif // ENABLE_NETWORKGATEWAY


TEST_F(SoftwareContainerApp, TestJobReturnCode) {

    auto jobTrue = getSc().createCommandJob("/bin/true");
    jobTrue->start();
    jobTrue->wait();
    ASSERT_TRUE(jobTrue->isSuccess());

    auto jobFalse = getSc().createCommandJob("/bin/false");
    jobFalse->start();
    jobFalse->wait();
    ASSERT_TRUE(jobFalse->isError());
}

/**
 * Checks that DBUS daemons are accessible if the corresponding capability is enabled
 */
TEST_F(SoftwareContainerApp, TestDBusGatewayWithAccess) {
    std::string configStr = "[{"
        "\"dbus-gateway-config-session\": "
           "[{ \"direction\": \"*\", "
              "\"interface\": \"*\", "
              "\"object-path\": \"*\", "
              "\"method\": \"*\" "
        "}], "
        "\"dbus-gateway-config-system\": "
           "[{ \"direction\": \"*\", "
              "\"interface\": \"*\", "
              "\"object-path\": \"*\", "
              "\"method\": \"*\" "
        "}]"
    "}]";
    startGateways(configStr, DBusGateway::ID);

    {
        auto jobTrue = getSc().createCommandJob(
                "/usr/bin/dbus-send --system --dest=org.freedesktop.DBus / org.freedesktop.DBus.Introspectable.Introspect");
        jobTrue->start();
        jobTrue->wait();
        ASSERT_TRUE(jobTrue->isSuccess());
    }

    {
        auto jobTrue = getSc().createCommandJob(
                "/usr/bin/dbus-send --session --dest=org.freedesktop.DBus / org.freedesktop.DBus.Introspectable.Introspect");
        jobTrue->start();
        jobTrue->wait();
        ASSERT_TRUE(jobTrue->isSuccess());
    }
}

// Regression test against previously reported bug.
TEST_F(SoftwareContainerApp, TestDBusGatewayOutputBuffer) {
    std::string configStr =
        "[{"
        "\"dbus-gateway-config-session\": "
           "[{ \"direction\": \"*\", "
              "\"interface\": \"*\", "
              "\"object-path\": \"*\", "
              "\"method\": \"*\" "
           "}], "
        "\"dbus-gateway-config-system\": "
           "[{ \"direction\": \"*\", "
              "\"interface\": \"*\", "
              "\"object-path\": \"*\", "
              "\"method\": \"*\" "
           "}]"
    "}]";
    startGateways(configStr, DBusGateway::ID);

    for(int i=0; i<2000; i++) {
        auto jobTrue = getSc().createCommandJob(
                "/usr/bin/dbus-send --session --dest=org.freedesktop.DBus / org.freedesktop.DBus.Introspectable.Introspect");
        jobTrue->start();
        jobTrue->wait();
        ASSERT_TRUE(jobTrue->isSuccess());
    }
}

/**
 * Checks that DBUS is not accessible if the corresponding capability is not enabled
 */
TEST_F(SoftwareContainerApp, TestDBusGatewayWithoutAccess) {

    {
        auto jobTrue = getSc().createCommandJob(
                "/usr/bin/dbus-send --session --print-reply --dest=org.freedesktop.DBus / org.freedesktop.DBus.Introspectable.Introspect");
        jobTrue->start();
        jobTrue->wait();
        ASSERT_TRUE(jobTrue->isError());
    }

    {
        auto jobTrue = getSc().createCommandJob(
                "/usr/bin/dbus-send --system --print-reply --dest=org.freedesktop.DBus / org.freedesktop.DBus.Introspectable.Introspect");
        jobTrue->start();

        // We expect the system bus to be accessible, even if we can not access any service. TODO : test if the services are accessible
        jobTrue->wait();
        ASSERT_TRUE(jobTrue->isError());
    }

}


/**
 * Test that supplying GatewayConfiguration containing gateway-ids that do not map to any existing gateway fail.
 */
TEST_F(SoftwareContainerApp, TestSetConfigurationBadID) {
    GatewayConfiguration gc;

    gc.append("GATEWAY_ID_THAT_DOES_NOT_MAP_TO_ANY_EXISTING_GATEWAY", "[{}]");

    ASSERT_THROW(m_sc->startGateways(gc), GatewayError);
}
