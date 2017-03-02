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

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <cassert>

#include "softwarecontainer_test.h"

#include "gateway/pulsegateway.h"
#include "gateway/waylandgateway.h"
#include "gateway/dbus/dbusgateway.h"
#include "gateway/network/networkgateway.h"
#include "gateway/files/filegateway.h"

#include "functionjob.h"
#include "commandjob.h"

LOG_DECLARE_DEFAULT_CONTEXT(defaultContext, "ff", "dd");

static constexpr const int SUCCESS = 0;
static constexpr const int FAILURE = 1;

class SoftwareContainerApp : public SoftwareContainerLibTest
{

public:

    bool startGateways(const std::string &config, const std::string &gatewayID)
    {
        json_error_t error;
        json_t *configJson = json_loads(config.c_str(), 0, &error);
        assert(configJson != nullptr);

        GatewayConfiguration gwConf;
        gwConf.append(gatewayID, configJson);

        bool result = getSc().startGateways(gwConf);

        json_decref(configJson);

        return result;
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

    /*
     * Wayland gateway tests use this as a helper to assert that:
     *
     *  * There is the expected environment variable set inside the container.
     *  * There is a wayland socket inside the container.
     */
    std::shared_ptr<FunctionJob> waylandTestHelperFunctionJob()
    {
        auto jobTrue = getSc().createFunctionJob([] () {
            int ERROR = 1;
            int SUCCESS = 0;

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

            if (!isSocket(socketPath)) {
                return ERROR;
            }

            return SUCCESS;
        });

        return jobTrue;
    }
};


/*
 * Wayland gateway test
 *
 * Test that a GW configuration that first applies a permissive config and then
 * a less permissive config results in the most permissive config being kept.
 *
 * The test asserts that:
 *  * An expected environment variable is set inside the container.
 *  * There is an expected socket created inside the container.
 *  * A call to weston-info inside the container is successful
 */
TEST_F(SoftwareContainerApp, TestWaylandWhitelist) {
    std::string configStr = "[{ \"enabled\" : true },\
                              { \"enabled\" : false }]";
    startGateways(configStr, WaylandGateway::ID);

    auto helperJob = waylandTestHelperFunctionJob();

    helperJob->start();
    helperJob->wait();

    ASSERT_TRUE(helperJob->isSuccess());

    auto westonJob = getSc().createCommandJob("/bin/sh -c \"/usr/bin/weston-info > /dev/null\"");

    westonJob->start();
    westonJob->wait();

    ASSERT_TRUE(westonJob->isSuccess());
}

/*
 * Wayland gateway test
 *
 * Similar test to the whitelisting test for non dynamic behavior, but this test
 * applies the configs in separate call sequences to setConfig and activate, i.e
 * the whitelisting behavior is tested when the gateway is reconfigured and reactivated
 * compared to whitelisting behavior when configuraion and activation happens just
 * once.
 */
TEST_F(SoftwareContainerApp, TestWaylandWhitelistingForDynamicBehavior) {
    std::string configStr = "[{ \"enabled\" : false}]";

    startGateways(configStr, WaylandGateway::ID);

    auto helperJob = waylandTestHelperFunctionJob();

    helperJob->start();
    helperJob->wait();

    // The gateway has not been enabled so this should not be a success
    ASSERT_FALSE(helperJob->isSuccess());

    auto westonJob = getSc().createCommandJob("/bin/sh -c \"/usr/bin/weston-info > /dev/null\"");

    westonJob->start();
    westonJob->wait();

    // The gateway has not been enabled so this should not be a success
    ASSERT_FALSE(westonJob->isSuccess());


    // Second part of the test, apply configs, activate, and to everything again...
    configStr = "[{ \"enabled\" : true }]";

    startGateways(configStr, WaylandGateway::ID);

    helperJob->start();
    helperJob->wait();

    ASSERT_TRUE(helperJob->isSuccess());

    westonJob = getSc().createCommandJob("/bin/sh -c \"/usr/bin/weston-info > /dev/null\"");

    westonJob->start();
    westonJob->wait();

    ASSERT_TRUE(westonJob->isSuccess());
}

/*
 * Wayland gateway test
 *
 * Test that the gateway can be configured and activated twice, i.e. the 'dynamic' behavior.
 * A call to startGateways leads to one call to setConfig() and to activate().
 */
TEST_F(SoftwareContainerApp, TestWaylandDynamicBehavior) {
    std::string configStr = "[{ \"enabled\" : true }]";
    ASSERT_TRUE(startGateways(configStr, WaylandGateway::ID));
    ASSERT_TRUE(startGateways(configStr, WaylandGateway::ID));
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

TEST_F(SoftwareContainerApp, FileGatewayReadOnly) {

    std::string tempFilename1 = createTempFile();

    // They will be mapped to these files, which should not yet exist
    std::string containerPath1 = "/tmp/testFile1";
    auto jobMounted = getSc().createFunctionJob([&] () {
        return isFile(containerPath1) ? SUCCESS : FAILURE;
    });
    jobMounted->start();
    jobMounted->wait();
    ASSERT_TRUE(jobMounted->isError());

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
    jobMounted->wait();
    ASSERT_TRUE(jobMounted->isSuccess());

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
        return isFile(containerPath1) ? SUCCESS : FAILURE;
    });
    jobMounted->start();
    jobMounted->wait();
    ASSERT_TRUE(jobMounted->isError());

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
    jobMounted->wait();
    ASSERT_TRUE(jobMounted->isSuccess());

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
        return isFile(tempFilename) ? SUCCESS : FAILURE;
    });
    job->start();
    job->wait();

    // File should not be available
    ASSERT_TRUE(job->isError());

    ASSERT_TRUE(bindMountInContainer(tempFilename, tempFilename, true));

    // Now file should be available
    job->start();
    job->wait();
    ASSERT_TRUE(job->isSuccess());

    // Test that we can mount to some place where there is something else
    // mounted higher up in the hierarchy (in this case, /dev)
    ASSERT_TRUE(bindMountInContainer("/dev/shm", "/dev/shm", false));
    auto jobShm = getSc().createFunctionJob([&] () {
        return isDirectory("/dev/shm") ? SUCCESS : FAILURE;
    });
    jobShm->start();
    jobShm->wait();
    ASSERT_TRUE(jobShm->isSuccess());
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
        return isFile(tempFilename) ? SUCCESS : FAILURE;
    });
    job->start();
    job->wait();
    ASSERT_TRUE(job->isError());

    // Bind mount the file
    ASSERT_TRUE(bindMountInContainer(tempFilename, tempFilename, true));

    // Check that the file is now in the container
    job->start();
    job->wait();
    ASSERT_TRUE(job->isSuccess());

    // Try to bind mount again. This should fail!
    ASSERT_FALSE(bindMountInContainer(tempFilename, tempFilename, true));

    // Check that the file is still in the container
    job->start();
    job->wait();
    ASSERT_TRUE(job->isSuccess());
}

/**
 * Test whether it should not be possible to mount over existing mounts
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
        return isDirectory(tempDirname) ? SUCCESS : FAILURE;
    });
    job1->start();
    job1->wait();
    ASSERT_TRUE(job1->isError());

    ASSERT_TRUE(bindMountInContainer(tempDirname, tempDirname, false));

    job1->start();
    job1->wait();
    ASSERT_TRUE(job1->isSuccess());

    // Test that an unexisting path is created when you mount to it
    std::string longUnexistingPath = "/var/apa/bepa/cepa";
    ASSERT_TRUE(bindMountInContainer(tempDirname, longUnexistingPath, false));
    auto job2 = getSc().createFunctionJob([&] () {
        return isDirectory(longUnexistingPath) ? SUCCESS : FAILURE;
    });
    job2->start();
    job2->wait();
    ASSERT_TRUE(job2->isSuccess());

    // Write some data to a file inside the directory
    std::string tempFilename = createTempFile(tempDirname);
    std::string content = "GFDGDFHDHRWG";

    writeToFile(tempFilename, content);

    auto job3 = getSc().createFunctionJob([&] () {
        return isFile(std::string(tempFilename)) ? SUCCESS : FAILURE;
    });
    job3->start();
    job3->wait();
    ASSERT_TRUE(job3->isSuccess());

    // Test that we can mount to some place where there is something else
    // mounted higher up in the hierarchy (in this case, /dev)
    ASSERT_TRUE(bindMountInContainer("/dev/shm", "/dev/shm", false));
    auto jobShm = getSc().createFunctionJob([&] () {
        return isDirectory("/dev/shm") ? SUCCESS : FAILURE;
    });
    jobShm->start();
    jobShm->wait();
    ASSERT_TRUE(jobShm->isSuccess());
}

/*
 * Test that POSIX SHM can be mounted into a container, and that objects created on the host
 * are accessible in the container after mounting, but only then.
 */
TEST_F(SoftwareContainerApp, TestPOSIXSHM) {
    std::string testData1 = "test data to be read";
    std::string testData2 = "read be to data test";
    std::string objName1 = "/testObject1";
    std::string objName2 = "/testObject2";

    // Create a first object
    int fd = shm_open(objName1.c_str(), O_CREAT | O_RDWR, 0666);
    ASSERT_NE(fd, -1);
    ASSERT_NE(write(fd, testData1.c_str(), testData1.length()), -1);
    ASSERT_EQ(close(fd), 0);

    // And create a second object
    fd = shm_open(objName2.c_str(), O_CREAT | O_RDWR, 0666);
    ASSERT_NE(fd, -1);
    ASSERT_NE(write(fd, testData2.c_str(), testData2.length()), -1);
    ASSERT_EQ(close(fd), 0);

    /*
     * Lambda function to read a shm object and compare it with some expected
     * test data.
     *
     * This returns true if the object exists and its content is equal to what was expected,
     * and returns false otherwise.
     */
    auto readMatchObject = [] (std::string objName, std::string testData) {
        int dataLength = testData.length();
        int fd = shm_open(objName.c_str(), O_RDONLY, 0666);
        if (fd == -1) {
            return false;
        }
        bool retval = true;
        char *buf = new char[dataLength];
        int bytes = read(fd, buf, dataLength);
        close(fd);

        retval = (bytes == dataLength) && strncmp(buf, testData.c_str(), dataLength) == 0;
        delete[] buf;
        return retval;
    };

    // Mount the first one, and check that it can be found
    ASSERT_TRUE(bindMountInContainer("/dev/shm/testObject1", "/dev/shm/testObject1", false));
    auto jobShm1 = getSc().createFunctionJob([objName1, testData1, readMatchObject] () {
        return readMatchObject(objName1, testData1) ? SUCCESS : FAILURE;
    });
    jobShm1->start();
    jobShm1->wait();
    ASSERT_TRUE(jobShm1->isSuccess());

    // Since the second one is not mounted, it should not be found.
    auto jobShm2 = getSc().createFunctionJob([objName2, testData2, readMatchObject] () {
        return readMatchObject(objName2, testData2) ? SUCCESS : FAILURE;
    });
    jobShm2->start();
    jobShm2->wait();
    ASSERT_TRUE(jobShm2->isError());

    // Then we mount the second object, and makes sure we find it.
    ASSERT_TRUE(bindMountInContainer("/dev/shm/testObject2", "/dev/shm/testObject2", false));
    jobShm2->start();
    jobShm2->wait();
    ASSERT_TRUE(jobShm2->isSuccess());
}


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

                return done == 1 ? SUCCESS : FAILURE;
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
            _exit(FAILURE);
        }

        remote.sun_family = AF_UNIX;
        strncpy(remote.sun_path, tempUnixSocket, strlen(tempUnixSocket) + 1);
        len = strlen(remote.sun_path) + sizeof(remote.sun_family);
        if (connect(s, (struct sockaddr *)&remote, len) == -1) {
            _exit(FAILURE);
        }


        if (send(s, str, strlen(str), 0) == -1) {
            _exit(FAILURE);
        }

        close(s);
        _exit(SUCCESS);
    }
    else //Parent process
    {
        job1->start();
        job1->wait();
        ASSERT_TRUE(job1->isSuccess());
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
        return isFile(soundFile) ? SUCCESS : FAILURE;
    });
    job1->start();
    job1->wait();
    ASSERT_TRUE(job1->isSuccess());

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
        auto job = getSc().createCommandJob(
                "/usr/bin/dbus-send --session --print-reply --dest=org.freedesktop.DBus / org.freedesktop.DBus.Introspectable.Introspect");
        job->start();
        job->wait();
        ASSERT_TRUE(job->isError());
    }

    {
        auto job = getSc().createCommandJob(
                "/usr/bin/dbus-send --system --print-reply --dest=org.freedesktop.DBus / org.freedesktop.DBus.Introspectable.Introspect");
        job->start();

        // We expect the system bus to be accessible, even if we can not access any service.
        // TODO : test if the services are accessible
        job->wait();
        ASSERT_TRUE(job->isError());
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
