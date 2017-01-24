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

    ReturnCode bindMountInContainer(const std::string src, const std::string dst, bool readOnly)
    {
        return getSc().getContainer()->bindMountInContainer(src, dst, readOnly);
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
        std::string socketPath = logging::StringBuilder() << waylandDir << "/"
                                                          << WaylandGateway::SOCKET_FILE_NAME;
        log_debug() << "isSocket : " << socketPath << " " << isSocket(socketPath);
        if ( !isSocket(socketPath) ) {
            return ERROR;
        }

        return SUCCESS;

    });
    jobTrue->start();
    ASSERT_EQ(jobTrue->wait(), 0);

    auto westonJob = getSc().createCommandJob("/bin/sh -c \"/usr/bin/weston-info > /dev/null\"");
    westonJob->start();
    ASSERT_EQ(westonJob->wait(), 0);

}

/*
 * Test that it is not possible to create two containers with the same id.
 */
TEST_F(SoftwareContainerApp, DoubleIDCreatesError) {
    const ContainerID id = 1;

    auto config = createConfig();
    SoftwareContainer s1(id, std::move(config));

    config = createConfig();
    SoftwareContainer s2(id, std::move(config));

    ASSERT_TRUE(isSuccess(s1.init()));
    ASSERT_TRUE(isError(s2.init()));
}

TEST_F(SoftwareContainerApp, EnvVarsSet) {
    auto job = getSc().createFunctionJob([&] () {
        bool hasTestVar = false;
        Glib::getenv("TESTVAR", hasTestVar);
        return hasTestVar ? 0 : 1;
    });
    job->setEnvironmentVariable("TESTVAR","YES");
    job->start();
    ASSERT_EQ(job->wait(), 0);
}

static constexpr int EXISTENT = 1;
static constexpr int NON_EXISTENT = 0;

TEST_F(SoftwareContainerApp, FileGatewayReadOnly) {

    bool directory = false;
    bool shouldUnlink = false;
    std::string tempFilename1 = getTempPath(directory, shouldUnlink);

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
    ASSERT_EQ(jobReadData->wait(), 0);

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

    // Remove the temp files
    ASSERT_EQ(unlink(tempFilename1.c_str()), 0);
}

TEST_F(SoftwareContainerApp, FileGatewayReadWrite) {

    bool directory = false;
    bool shouldUnlink = false;
    std::string tempFilename1 = getTempPath(directory, shouldUnlink);

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
    ASSERT_EQ(jobReadData->wait(), 0);

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

    // Let's remove the temp files also
    ASSERT_EQ(unlink(tempFilename1.c_str()), 0);
}

/**
 * Test whether the mounting of files works properly
 */
TEST_F(SoftwareContainerApp, TestFileMounting) {

    char tempFilename[] = "/tmp/blablaXXXXXX";
    ASSERT_NE(mkstemp(tempFilename), -1);

    std::string content = "GFDGDFHDHRWG";
    writeToFile(tempFilename, content);

    auto job = getSc().createFunctionJob([&] () {
        return isFile(tempFilename) ? EXISTENT : NON_EXISTENT;
    });
    job->start();

    // File should not be available
    ASSERT_EQ(job->wait(), NON_EXISTENT);

    ASSERT_TRUE(isSuccess(bindMountInContainer(tempFilename, tempFilename, true)));

    // Now file should be available
    job->start();
    ASSERT_EQ(job->wait(), EXISTENT);
}

/**
 * Test that it is not possible to mount to same place twice
 */
TEST_F(SoftwareContainerApp, TestDoubleMounting) {

    char tempFilename[] = "/tmp/blablaXXXXXX";
    int fileDescriptor = mkstemp(tempFilename);
    ASSERT_NE(fileDescriptor, -1);

    // create a temporary file with some content
    const char *content = "GFDGDFHDHRWG";
    write(fileDescriptor, content, sizeof(content));
    close(fileDescriptor);

    // Make sure that the file is not already in the container
    auto job = getSc().createFunctionJob([&] () {
        return isFile(tempFilename) ? EXISTENT : NON_EXISTENT;
    });
    job->start();
    ASSERT_EQ(job->wait(), NON_EXISTENT);

    // Bind mount the file
    ASSERT_TRUE(isSuccess(bindMountInContainer(tempFilename, tempFilename, true)));

    // Check that the file is now in the container
    job->start();
    ASSERT_EQ(job->wait(), EXISTENT);

    // Try to bind mount again. This should fail!
    ASSERT_TRUE(isError(bindMountInContainer(tempFilename, tempFilename, true)));

    // Check that the file is still in the container
    job->start();
    ASSERT_EQ(job->wait(), EXISTENT);
}

/**
 * Test whether it should not be possible to mount over LXC mounts
 */
TEST_F(SoftwareContainerApp, TestMountingOverRoot) {

    char tempDirname[] = "/tmp/blablaXXXXXX";
    mkdtemp(tempDirname);
    ASSERT_TRUE(isDirectory(tempDirname));

    ASSERT_TRUE(isError(bindMountInContainer(tempDirname, "/", false)));
    ASSERT_TRUE(isError(bindMountInContainer(tempDirname, "/lib", false)));
}

/**
 * Test whether the mounting of folder works properly
 */
TEST_F(SoftwareContainerApp, TestFolderMounting) {

    char tempDirname[] = "/tmp/blablaXXXXXX";
    mkdtemp(tempDirname);
    ASSERT_TRUE(isDirectory(tempDirname));

    auto job1 = getSc().createFunctionJob([&] () {
        return isDirectory(tempDirname) ? EXISTENT : NON_EXISTENT;
    });
    job1->start();
    ASSERT_EQ(job1->wait(), NON_EXISTENT);

    ASSERT_TRUE(isSuccess(bindMountInContainer(tempDirname, tempDirname, false)));

    job1->start();
    ASSERT_EQ(job1->wait(), EXISTENT);
    //
    // Test that an unexisting path is created when you mount to it
    const char *longUnexistingPath = "/var/apa/bepa/cepa";
    ASSERT_TRUE(isSuccess(bindMountInContainer(tempDirname, longUnexistingPath, false)));
    auto job2 = getSc().createFunctionJob([&] () {
        return isDirectory(longUnexistingPath) ? EXISTENT : NON_EXISTENT;
    });
    job2->start();
    ASSERT_EQ(job2->wait(), EXISTENT);

    // Write some data to a file inside the directory
    char *tempFilename = strcat(tempDirname, "/bluhuXXXXXX");
    std::string content = "GFDGDFHDHRWG";
    ASSERT_NE(mkstemp(tempFilename), -1);

    writeToFile(tempFilename, content);
    ASSERT_TRUE(isFile(tempFilename));

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

    char tempDirname[] = "/tmp/blablaXXXXXX";
    ASSERT_FALSE(mkdtemp(tempDirname) == NULL);

    ASSERT_TRUE(isDirectory(tempDirname));
    ASSERT_TRUE(isSuccess(bindMountInContainer(tempDirname, tempDirname, false)));

    char *tmp = new char[strlen(tempDirname) + 8];
    strcpy(tmp, tempDirname);
    char *tempUnixSocket = strcat(tmp, "/socket");

    auto job1 = getSc().createFunctionJob([&] () {

                int fd, fd2, done, n;
                char str[100];
                socklen_t t;
                sockaddr_un local, remote;
                local.sun_family = AF_UNIX;
                strcpy(local.sun_path, tempUnixSocket);
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

        char *x = new char[strlen(tempDirname) + 8];
        strcpy(x, tempDirname);
        strcat(x, "/socket");

        remote.sun_family = AF_UNIX;
        strcpy(remote.sun_path, x);
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


TEST_F(SoftwareContainerApp, MultithreadTest) {

    static const int TIMEOUT = 20;
    static const ContainerID containerID = 1;

    auto config = createConfig();
    SoftwareContainer lib(containerID, std::move(config));

    bool finished = false;

    auto f = [&]() {
        log_info() << "Initializing";
        lib.init();
        finished = true;
    };

    std::thread t(f);

    for (int i = 0; (i < TIMEOUT) && !finished; i++) {
        log_info() << "Waiting for softwarecontainer to be initialized";
        sleep(1);
    }

    ASSERT_TRUE(finished);

    if (finished) {
        t.join();
    }

}

TEST_F(SoftwareContainerApp, DISABLED_TestPulseAudioEnabled) {

    std::string configStr = "[ { \"audio\" : true } ]";
    startGateways(configStr, PulseGateway::ID);

    // We need access to the test file, so we bind mount it
    std::string soundFile = std::string(TEST_DATA_DIR) + std::string("/Rear_Center.wav");
    ASSERT_TRUE(isSuccess(bindMountInContainer(soundFile, soundFile, true)));

    // Make sure the file is there
    auto job1 = getSc().createFunctionJob([&] () {
        return isFile(soundFile) ? EXISTENT : NON_EXISTENT;
    });
    job1->start();
    ASSERT_EQ(job1->wait(), EXISTENT);

    auto job2 = getSc().createCommandJob("/usr/bin/paplay " + soundFile);
    job2->start();
    ASSERT_TRUE(job2->isRunning());
    ASSERT_EQ(job2->wait(), 0);
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
    ASSERT_NE(job->wait(), 0);

    auto job2 = getSc().createCommandJob("/bin/sh -c \"ping 8.8.8.8 -c 5 -q > /dev/null 2>&1\"");
    job2->start();
    ASSERT_NE(job2->wait(), 0);
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
    ASSERT_NE(job->wait(), 0);

    auto job2 = getSc().createCommandJob("/bin/sh -c \"ping 8.8.8.8 -c 5 -q > /dev/null 2>&1\"");
    job2->start();
    ASSERT_NE(job2->wait(), 0);
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
    ASSERT_EQ(job->wait(), 0);
}

#endif // ENABLE_NETWORKGATEWAY


TEST_F(SoftwareContainerApp, TestJobReturnCode) {

    auto jobTrue = getSc().createCommandJob("/bin/true");
    jobTrue->start();
    ASSERT_EQ(jobTrue->wait(), 0);

    auto jobFalse = getSc().createCommandJob("/bin/false");
    jobFalse->start();
    ASSERT_NE(jobFalse->wait(), 0);
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
        ASSERT_EQ(jobTrue->wait(), 0);
    }

    {
        auto jobTrue = getSc().createCommandJob(
                "/usr/bin/dbus-send --session --dest=org.freedesktop.DBus / org.freedesktop.DBus.Introspectable.Introspect");
        jobTrue->start();
        ASSERT_EQ(jobTrue->wait(), 0);
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
        ASSERT_EQ(jobTrue->wait(), 0);
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

        ASSERT_NE(jobTrue->wait(), 0);
    }

    {
        auto jobTrue = getSc().createCommandJob(
                "/usr/bin/dbus-send --system --print-reply --dest=org.freedesktop.DBus / org.freedesktop.DBus.Introspectable.Introspect");
        jobTrue->start();

        // We expect the system bus to be accessible, even if we can not access any service. TODO : test if the services are accessible
        ASSERT_NE(jobTrue->wait(), 0);
    }

}

