/*
 * Copyright (C) 2016 Pelagicore AB
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

    void startGateways(const GatewayConfiguration &config)
    {
        getSc().startGateways(config);
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
        return *sc;
    }
};

TEST_F(SoftwareContainerApp, TestWaylandWhitelist) {

    GatewayConfiguration config;
    std::string configStr = "[ { \"enabled\" : true } ]";
    std::string configStrFalse = "[ { \"enabled\" : false } ]";
    json_error_t error;
    json_t *configJson = json_loads(configStr.c_str(), 0, &error);
    ASSERT_FALSE(configJson == nullptr);
    config.append(WaylandGateway::ID ,configJson);

    configJson = json_loads(configStrFalse.c_str(), 0, &error);
    config.append(WaylandGateway::ID ,configJson);
    startGateways(config);
    json_decref(configJson);

    FunctionJob jobTrue(getSc(), [] (){
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
    jobTrue.start();
    ASSERT_EQ(jobTrue.wait(), 0);

    CommandJob westonJob(getSc(),"/bin/sh -c \"/usr/bin/weston-info > /dev/null\"");
    westonJob.start();
    ASSERT_EQ(westonJob.wait(), 0);

}

/*
 * Test that it is not possible to create two containers with the same id.
 */
TEST_F(SoftwareContainerApp, DoubleIDCreatesError) {

    std::shared_ptr<Workspace> workspace = std::make_shared<Workspace>();

    /* Set up the workspace with all the config values it needs.
     *
     * NOTE: This could be done more nicely perhaps, but the workspace will get an overhaul
     *       or be removed, so pending that design change this is a workaround.
     */
    workspace->m_enableWriteBuffer = false;
    workspace->m_containerRootDir = SHARED_MOUNTS_DIR_TESTING; // Should be set be CMake
    workspace->m_containerConfigPath = LXC_CONFIG_PATH_TESTING; // Should be set be CMake
    workspace->m_containerShutdownTimeout = 1;

    const ContainerID id = 1;

    SoftwareContainer s1(workspace, id);
    SoftwareContainer s2(workspace, id);

    s1.setMainLoopContext(getMainContext());
    s2.setMainLoopContext(getMainContext());

    ASSERT_TRUE(isSuccess(s1.init()));
    ASSERT_TRUE(isError(s2.init()));
}

TEST_F(SoftwareContainerApp, EnvVarsSet) {
    FunctionJob job(getSc(), [&] () {
        bool hasTestVar = false;
        Glib::getenv("TESTVAR", hasTestVar);
        return hasTestVar ? 0 : 1;
    });
    job.setEnvironmentVariable("TESTVAR","YES");
    job.start();
    ASSERT_EQ(job.wait(), 0);
}

static constexpr int EXISTENT = 1;
static constexpr int NON_EXISTENT = 0;

TEST_F(SoftwareContainerApp, FileGatewayReadOnly) {

    // Make sure /tmp exists in both host and container
    ASSERT_TRUE(isDirectory("/tmp"));
    FunctionJob job0(getSc(), [&] () {
        return isDirectory("/tmp") ? EXISTENT : NON_EXISTENT;
    });
    job0.start();
    ASSERT_EQ(job0.wait(), EXISTENT);

    // Create a temporary file, and verify that we could indeed create it.
    char tempFilename1[] = "/tmp/fileGatewayXXXXXX";
    int fd1 = mkstemp(tempFilename1);
    ASSERT_NE(fd1, 0);
    close(fd1);

    // They will be mapped to these files, which should not yet exist
    std::string containerPath1 = "/tmp/testFile1";
    FunctionJob jobMounted(getSc(), [&] () {
        return isFile(containerPath1) ? EXISTENT : NON_EXISTENT;
    });
    jobMounted.start();
    ASSERT_EQ(jobMounted.wait(), NON_EXISTENT);

    // Let's configure the env gateway
    GatewayConfiguration config;
    std::string configStr =
    "["
        "{"
            "  \"path-host\" : \"" + std::string(tempFilename1) + "\""
            ", \"path-container\" : \"" + containerPath1 + "\""
            ", \"read-only\": true"
        "}"
    "]";
    json_error_t error;
    json_t *configJson = json_loads(configStr.c_str(), 0, &error);
    ASSERT_FALSE(configJson == nullptr);
    config.append(FileGateway::ID, configJson);
    startGateways(config);
    json_decref(configJson);

    jobMounted.start();
    ASSERT_EQ(jobMounted.wait(), EXISTENT);

    // Write some data to the files outside the container and make sure we can
    // read it inside the container
    std::string testData = "testdata";
    writeToFile(tempFilename1, testData);

    // Test if we can read the data back into a variable
    FunctionJob jobReadData(getSc(), [&] () {
        std::string readBack;
        readFromFile(containerPath1, readBack);
        return readBack == testData ? 0 : 1;
    });
    jobReadData.start();
    ASSERT_EQ(jobReadData.wait(), 0);

    // Make sure we can't write to the file
    std::string badData = "This data should never be read";
    FunctionJob jobWriteDataRO(getSc(), [&] () {
        writeToFile(containerPath1, badData);
        return 0;
    });
    jobWriteDataRO.start();
    jobWriteDataRO.wait();

    std::string readBack = "";
    readFromFile(tempFilename1, readBack);
    ASSERT_EQ(readBack, testData);

    // Remove the temp files
    ASSERT_EQ(unlink(tempFilename1), 0);
}

TEST_F(SoftwareContainerApp, FileGatewayReadWrite) {
    // Make sure /tmp exists in both host and container
    ASSERT_TRUE(isDirectory("/tmp"));
    FunctionJob job0(getSc(), [&] () {
        return isDirectory("/tmp") ? EXISTENT : NON_EXISTENT;
    });
    job0.start();
    ASSERT_EQ(job0.wait(), EXISTENT);

    // Create one temporary file, and verify that we indeed could create it.
    char tempFilename1[] = "/tmp/fileGatewayXXXXXX";
    int fd1 = mkstemp(tempFilename1);
    ASSERT_NE(fd1, 0);
    close(fd1);

    // They will be mapped to these files
    std::string containerPath1 = "/tmp/testFile1";
    FunctionJob jobMounted(getSc(), [&] () {
        return isFile(containerPath1) ? EXISTENT : NON_EXISTENT;
    });
    jobMounted.start();
    ASSERT_EQ(jobMounted.wait(), NON_EXISTENT);

    GatewayConfiguration config;
    std::string configStr =
    "["
        "{" // The files below are mounted read-only
            "  \"path-host\" : \"" + std::string(tempFilename1) + "\""
            ", \"path-container\" : \"" + containerPath1 + "\""
            ", \"read-only\": false"
        "}"
    "]";
    json_error_t error;
    json_t *configJson = json_loads(configStr.c_str(), 0, &error);
    ASSERT_FALSE(configJson == nullptr);
    config.append(FileGateway::ID, configJson);
    startGateways(config);
    json_decref(configJson);

    jobMounted.start();
    ASSERT_EQ(jobMounted.wait(), EXISTENT);

    // Write some data to the files outside the container and make sure we can
    // read it inside the container
    std::string testData = "testdata";
    writeToFile(tempFilename1, testData);

    // Test if we can read the data back into a variable
    FunctionJob jobReadData(getSc(), [&] () {
        std::string readBack;
        readFromFile(containerPath1, readBack);
        return readBack == testData ? 0 : 1;
    });
    jobReadData.start();
    ASSERT_EQ(jobReadData.wait(), 0);

    // Make sure we can write to the file
    std::string newData = "This data should have been written";
    FunctionJob jobWriteData(getSc(), [&] () {
        writeToFile(containerPath1, newData);
        return 0;
    });
    jobWriteData.start();
    jobWriteData.wait();

    std::string readBack = "";
    readFromFile(tempFilename1, readBack);
    ASSERT_EQ(newData, readBack);

    // Let's remove the temp files also
    ASSERT_EQ(unlink(tempFilename1), 0);
}

/**
 * Test whether the mounting of files works properly
 */
TEST_F(SoftwareContainerApp, TestFileMounting) {

    char tempFilename[] = "/tmp/blablaXXXXXX";
    int fileDescriptor = mkstemp(tempFilename);

    ASSERT_NE(fileDescriptor, 0);

    // create a temporary file with some content
    const char *content = "GFDGDFHDHRWG";
    write(fileDescriptor, content, sizeof(content));
    close(fileDescriptor);

    FunctionJob job1(getSc(), [&] () {
        return isFile(tempFilename) ? EXISTENT : NON_EXISTENT;
    });
    job1.start();
    ASSERT_EQ(job1.wait(), NON_EXISTENT);

    ASSERT_TRUE(isSuccess(bindMountInContainer(tempFilename, tempFilename, true)));

    FunctionJob job2(getSc(), [&] () {
        return isFile(tempFilename) ? EXISTENT : NON_EXISTENT;
    });
    job2.start();
    ASSERT_EQ(job2.wait(), EXISTENT);

}

/**
 * Test that it is not possible to mount to same place twice
 */
TEST_F(SoftwareContainerApp, TestDoubleMounting) {

    char tempFilename[] = "/tmp/blablaXXXXXX";
    int fileDescriptor = mkstemp(tempFilename);

    ASSERT_NE(fileDescriptor, 0);

    // create a temporary file with some content
    const char *content = "GFDGDFHDHRWG";
    write(fileDescriptor, content, sizeof(content));
    close(fileDescriptor);

    // Make sure that the file is not already in the container
    FunctionJob job(getSc(), [&] () {
        return isFile(tempFilename) ? EXISTENT : NON_EXISTENT;
    });
    job.start();
    ASSERT_EQ(job.wait(), NON_EXISTENT);

    // Bind mount the file
    ASSERT_TRUE(isSuccess(bindMountInContainer(tempFilename, tempFilename, true)));

    // Check that the file is now in the container

    job.start();
    ASSERT_EQ(job.wait(), EXISTENT);

    // Try to bind mount again. This should fail!
    ASSERT_TRUE(isError(bindMountInContainer(tempFilename, tempFilename, true)));

    // Check that the file is still in the container

    job.start();
    ASSERT_EQ(job.wait(), EXISTENT);

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

    FunctionJob job1(getSc(), [&] () {
        return isDirectory(tempDirname) ? EXISTENT : NON_EXISTENT;
    });
    job1.start();
    ASSERT_EQ(job1.wait(), NON_EXISTENT);

    ASSERT_TRUE(isSuccess(bindMountInContainer(tempDirname, tempDirname, false)));

    FunctionJob job2(getSc(), [&] () {
        return isDirectory(tempDirname) ? EXISTENT : NON_EXISTENT;
    });
    job2.start();
    ASSERT_EQ(job2.wait(), EXISTENT);

    // Write some data to a file inside the directory
    char *tempFilename = strcat(tempDirname, "/bluhuXXXXXX");
    int fileDescriptor = mkstemp(tempFilename);
    const char *content = "GFDGDFHDHRWG";
    write(fileDescriptor, content, sizeof(content));
    close(fileDescriptor);
    ASSERT_TRUE(isFile(tempFilename));

    FunctionJob job3(getSc(), [&] () {
                std::string td(tempDirname);
                return isFile(td) ? EXISTENT : NON_EXISTENT;
            });
    job3.start();
    ASSERT_EQ(job3.wait(), EXISTENT);
}

#include <stdlib.h>

/**
 * Test whether the mounting of sockets works properly
 */
TEST_F(SoftwareContainerApp, TestUnixSocket) {

    char tempFilename[] = "/tmp/blablaXXXXXX";
    char *tempDirname = mkdtemp(tempFilename);

    ASSERT_TRUE(isDirectory(tempDirname));
    ASSERT_TRUE(isSuccess(bindMountInContainer(tempDirname, tempDirname, false)));

    char *tmp = new char[strlen(tempDirname) + 8];
    strcpy(tmp, tempDirname);
    char *tempUnixSocket = strcat(tmp, "/socket");

    FunctionJob job1(getSc(), [&] () {

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
        job1.start();
        ASSERT_EQ(job1.wait(), EXISTENT);
    }
}


TEST(SoftwareContainer, MultithreadTest) {

    static const int TIMEOUT = 20;

    std::shared_ptr<Workspace> workspacePtr(new Workspace());
    SoftwareContainer lib(workspacePtr, 2);
    lib.setMainLoopContext(Glib::MainContext::get_default());

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

    GatewayConfiguration config;
    std::string configStr = "[ { \"audio\" : true } ]";
    json_error_t error;
    json_t *configJson = json_loads(configStr.c_str(), 0, &error);
    ASSERT_FALSE(configJson == nullptr);
    config.append(PulseGateway::ID, configJson);
    startGateways(config);
    json_decref(configJson);

    // We need access to the test file, so we bind mount it
    std::string soundFile = std::string(TEST_DATA_DIR) + std::string("/Rear_Center.wav");
    ASSERT_TRUE(isSuccess(bindMountInContainer(soundFile, soundFile, true)));

    // Make sure the file is there
    FunctionJob job1(getSc(), [&] () {
        return isFile(soundFile) ? EXISTENT : NON_EXISTENT;
    });
    job1.start();
    ASSERT_EQ(job1.wait(), EXISTENT);

    CommandJob job2(getSc(), "/usr/bin/paplay " + soundFile);
    job2.start();
    ASSERT_TRUE(job2.isRunning());
    ASSERT_EQ(job2.wait(), 0);
}


TEST_F(SoftwareContainerApp, TestStdin) {
    CommandJob job(getSc(), "/bin/cat");
    job.captureStdin();
    job.captureStdout();
    job.start();
    ASSERT_TRUE(job.isRunning());

    const char outputBytes[] = "test string";
    char inputBytes[sizeof(outputBytes)] = {};

    unsigned int writtenBytesCount = write(job.stdin(), outputBytes, sizeof(outputBytes));
    ASSERT_EQ(writtenBytesCount, sizeof(outputBytes));

    unsigned int readBytesCount = read(job.stdout(), inputBytes, sizeof(inputBytes));
    ASSERT_EQ(readBytesCount, sizeof(outputBytes));

    SignalConnectionsHandler connections;
    addProcessListener(connections, job.pid(), [&] (
                int pid, int exitCode) {
                log_debug() << "Finished process:" << job.toString();
                log_debug() << "Pid was " << pid << ", exitCode was " << exitCode;
                exit();
            }, getMainContext());

    kill(job.pid(), SIGTERM);

    run();
}


/**
 * We do not enable the network gateway so we expect the ping to fail
 */
TEST_F(SoftwareContainerApp, TestNetworkInternetCapabilityDisabled) {
    CommandJob job(getSc(), "/bin/sh -c \"ping www.google.com -c 5 -q > /dev/null 2>&1\"");
    job.start();
    ASSERT_NE(job.wait(), 0);

    CommandJob job2(getSc(), "/bin/sh -c \"ping 8.8.8.8 -c 5 -q > /dev/null 2>&1\"");
    job2.start();
    ASSERT_NE(job2.wait(), 0);
}

/**
 * We can also disable it explicitly
 */
TEST_F(SoftwareContainerApp, TestNetworkInternetCapabilityDisabledExplicit) {
    GatewayConfiguration config;
    std::string configStr =
        "[{"
            "\"direction\": \"OUTGOING\","
            "\"allow\": []"
        "}]";
    json_error_t error;
    json_t *configJson = json_loads(configStr.c_str(), 0, &error);
    ASSERT_FALSE(configJson == nullptr);
    config.append(NetworkGateway::ID, configJson);
    startGateways(config);

    CommandJob job(getSc(), "/bin/sh -c \"ping www.google.com -c 5 -q > /dev/null 2>&1\"");
    job.start();
    ASSERT_NE(job.wait(), 0);

    CommandJob job2(getSc(), "/bin/sh -c \"ping 8.8.8.8 -c 5 -q > /dev/null 2>&1\"");
    job2.start();
    ASSERT_NE(job2.wait(), 0);
}

/**
 * This test checks that an external is accessible after the network gateway has been enabled to access the internet.
 */
TEST_F(SoftwareContainerApp, TestNetworkInternetCapabilityEnabled) {

    GatewayConfiguration config;
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
    json_error_t error;
    json_t *configJson = json_loads(configStr.c_str(), 0, &error);
    ASSERT_FALSE(configJson == nullptr);
    config.append(NetworkGateway::ID, configJson);
    startGateways(config);
    json_decref(configJson);

    CommandJob job(getSc(), "/bin/sh -c \"ping example.com -c 5 -q > /dev/null\"");
    job.start();
    ASSERT_EQ(job.wait(), 0);
}


TEST_F(SoftwareContainerApp, TestJobReturnCode) {

    CommandJob jobTrue(getSc(), "/bin/true");
    jobTrue.start();
    ASSERT_EQ(jobTrue.wait(), 0);

    CommandJob jobFalse(getSc(), "/bin/false");
    jobFalse.start();
    ASSERT_NE(jobFalse.wait(), 0);
}

/**
 * Checks that DBUS daemons are accessible if the corresponding capability is enabled
 */
TEST_F(SoftwareContainerApp, TestDBusGatewayWithAccess) {
    GatewayConfiguration config;
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
    json_error_t error;
    json_t *configJson = json_loads(configStr.c_str(), 0, &error);
    ASSERT_FALSE(configJson == nullptr);
    config.append(DBusGateway::ID, configJson);
    startGateways(config);
    json_decref(configJson);

    {
        CommandJob jobTrue(
                getSc(),
                "/usr/bin/dbus-send --system --dest=org.freedesktop.DBus / org.freedesktop.DBus.Introspectable.Introspect");
        jobTrue.start();
        ASSERT_EQ(jobTrue.wait(), 0);
    }

    {
        CommandJob jobTrue(
                getSc(),
                "/usr/bin/dbus-send --session --dest=org.freedesktop.DBus / org.freedesktop.DBus.Introspectable.Introspect");
        jobTrue.start();
        ASSERT_EQ(jobTrue.wait(), 0);
    }
}

// Regression test against previously reported bug.
TEST_F(SoftwareContainerApp, TestDBusGatewayOutputBuffer) {
    GatewayConfiguration config;
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
    json_error_t error;
    json_t *configJson = json_loads(configStr.c_str(), 0, &error);
    ASSERT_FALSE(configJson == nullptr);
    config.append(DBusGateway::ID, configJson);
    startGateways(config);
    json_decref(configJson);

    for(int i=0; i<2000; i++) {
        CommandJob jobTrue(
                getSc(),
                "/usr/bin/dbus-send --session --dest=org.freedesktop.DBus / org.freedesktop.DBus.Introspectable.Introspect");
        jobTrue.start();
        ASSERT_EQ(jobTrue.wait(), 0);
    }
}

/**
 * Checks that DBUS is not accessible if the corresponding capability is not enabled
 */
TEST_F(SoftwareContainerApp, TestDBusGatewayWithoutAccess) {

    {
        CommandJob jobTrue(
                getSc(),
                "/usr/bin/dbus-send --session --print-reply --dest=org.freedesktop.DBus / org.freedesktop.DBus.Introspectable.Introspect");
        jobTrue.start();

        ASSERT_NE(jobTrue.wait(), 0);
    }

    {
        CommandJob jobTrue(
                getSc(),
                "/usr/bin/dbus-send --system --print-reply --dest=org.freedesktop.DBus / org.freedesktop.DBus.Introspectable.Introspect");
        jobTrue.start();

        // We expect the system bus to be accessible, even if we can not access any service. TODO : test if the services are accessible
        ASSERT_NE(jobTrue.wait(), 0);
    }

}


TEST_F(SoftwareContainerApp, InitTest) {
    ASSERT_TRUE(getSc().isInitialized());
}
