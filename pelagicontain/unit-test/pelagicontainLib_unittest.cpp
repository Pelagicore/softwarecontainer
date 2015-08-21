/*
 *   Copyright (C) 2014 Pelagicore AB
 *   All rights reserved.
 */

#include <thread>
#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "generators.h"
#include "pelagicontain-lib.h"

#include "dbusgateway.h"
#include "waylandgateway.h"
#include "networkgateway.h"
#include "pulsegateway.h"


LOG_DECLARE_DEFAULT_CONTEXT(defaultContext, "ff", "dd");

class PelagicontainApp :
    public::testing::Test
{

public:
    PelagicontainApp()
    {
    }

    void SetUp() override
    {
        ::testing::Test::SetUp();
        lib = std::unique_ptr<PelagicontainLib> ( new PelagicontainLib() );
        lib->setContainerIDPrefix("Test-");
        lib->setMainLoopContext(m_context);
        ASSERT_TRUE( isSuccess( lib->init() ) );
    }

    void TearDown() override
    {
        ::testing::Test::TearDown();  // Remember to tear down the base fixture after cleaning up FooTest!
    }

    void run()
    {
        m_ml = Glib::MainLoop::create(m_context);
        m_ml->run();
    }

    void exit()
    {
        m_ml->quit();
    }

    void setGatewayConfigs(const GatewayConfiguration &config)
    {
        getLib().getPelagicontain().setGatewayConfigs(config);
    }

    Glib::RefPtr<Glib::MainContext> getMainContext()
    {
        return m_context;
    }

    void openTerminal()
    {
        lib->openTerminal("konsole -e");
    }

    PelagicontainLib &getLib()
    {
        return *lib;
    }

    Glib::RefPtr<Glib::MainContext> m_context = Glib::MainContext::get_default();
    Glib::RefPtr<Glib::MainLoop> m_ml;
    std::unique_ptr<PelagicontainLib> lib;
};



//
//TEST_F(PelagicontainApp, TestWayland) {
//
//    GatewayConfiguration config;
//    config[WaylandGateway::ID] = "[ { \"enabled\" : true } ]";
//
//    getLib().getPelagicontain().setGatewayConfigs(config);
//
//    //		openTerminal();
//    //		sleep(10000);
//
//    FunctionJob jobTrue(getLib(), [] (){
//
//                            bool ERROR = 1;
//                            bool SUCCESS = 0;
//
//                            const char *waylandDir = getenv("XDG_RUNTIME_DIR");
//
//                            log_debug() << "Wayland dir : " << waylandDir;
//
//                            if (waylandDir == nullptr)
//                                return ERROR;
//
//                            std::string socketPath = StringBuilder() << waylandDir << "/" << WaylandGateway::SOCKET_FILE_NAME;
//
//                            log_debug() << "isSocket : " << socketPath << " " << isSocket(socketPath);
//
//                            if ( !isSocket(socketPath) )
//                                return ERROR;
//
//                            return SUCCESS;
//
//                        });
//    jobTrue.start();
//
//    ASSERT_TRUE(jobTrue.wait() == 0);
//
//    //		CommandJob westonJob(lib,
//    //				"/usr/bin/weston-terminal");
//    //		westonJob.start();
//    //
//    //		ASSERT_TRUE(westonJob.wait() == 0);
//
//}


TEST_F(PelagicontainApp, FileGateway) {

    ASSERT_TRUE( isDirectory("/tmp") );

    char tempFilename[] = "/tmp/blablaXXXXXX";
    int fileDescriptor = mkstemp(tempFilename);
    close(fileDescriptor);
    ASSERT_TRUE( isFile(tempFilename) );
    ASSERT_FALSE( isDirectory(tempFilename) );
    ASSERT_FALSE( isSocket(tempFilename) );
    ASSERT_TRUE(unlink(tempFilename) == 0);

    const char *socketPath = "/run/user/1000/X11-display";
    ASSERT_TRUE( isSocket(socketPath) );
    ASSERT_FALSE( isFile(socketPath) );
    ASSERT_FALSE( isDirectory(socketPath) );

    const char *unexistingFile = "/run/user/10jhgj00/X11-dgfdgdagisplay";
    ASSERT_FALSE( isSocket(unexistingFile) );
    ASSERT_FALSE( isFile(unexistingFile) );
    ASSERT_FALSE( isDirectory(unexistingFile) );

}

static constexpr int EXISTENT = 1;
static constexpr int NON_EXISTENT = 0;


/**
 * Test whether the mounting of files works properly
 */
TEST_F(PelagicontainApp, TestFileMounting) {

    char tempFilename[] = "/tmp/blablaXXXXXX";
    int fileDescriptor = mkstemp(tempFilename);

    ASSERT_TRUE(fileDescriptor != 0);

    // create a temporary file with some content
    const char *content = "GFDGDFHDHRWG";
    write( fileDescriptor, content, sizeof(content) );
    close(fileDescriptor);

    FunctionJob job1(getLib(), [&] () {
                return isFile(tempFilename) ? EXISTENT : NON_EXISTENT;
            });
    job1.start();
    ASSERT_TRUE(job1.wait() == NON_EXISTENT);

    auto pathInContainer = getLib().getContainer().bindMountFileInContainer(tempFilename, basename( strdup(tempFilename) ), true);

    FunctionJob job2(getLib(), [&] () {
                return isFile(pathInContainer) ? EXISTENT : NON_EXISTENT;
            });
    job2.start();
    ASSERT_TRUE(job2.wait() == EXISTENT);

}


/**
 * Test whether the mounting of folder works properly
 */
TEST_F(PelagicontainApp, TestFolderMounting) {

    char tempFilename[] = "/tmp/blablaXXXXXX";
    mkdtemp(tempFilename);

    ASSERT_TRUE( isDirectory(tempFilename) );

    FunctionJob job1(getLib(), [&] () {
                return isDirectory(tempFilename) ? EXISTENT : NON_EXISTENT;
            });
    job1.start();
    ASSERT_TRUE(job1.wait() == NON_EXISTENT);

    auto pathInContainer = getLib().getContainer().bindMountFolderInContainer(tempFilename, basename( strdup(
                    tempFilename) ), true);

    FunctionJob job2(getLib(), [&] () {
                return isDirectory(pathInContainer) ? EXISTENT : NON_EXISTENT;
            });
    job2.start();
    ASSERT_TRUE(job2.wait() == EXISTENT);
}



TEST(PelagicontainLib, MultithreadTest) {

    static const int TIMEOUT = 20;

    PelagicontainLib lib;
    lib.setMainLoopContext( Glib::MainContext::get_default() );

    bool finished = false;

    auto f = [&]() {
        log_info() << "Initializing";
        lib.init();
        finished = true;
    };

    std::thread t(f);

    for (int i = 0; (i < TIMEOUT) && !finished; i++) {
        log_info() << "Waiting for pelagicontain to be initialized";
        sleep(1);
    }

    ASSERT_TRUE(finished);

    if (finished) {
        t.join();
    }

}


TEST_F(PelagicontainApp, TestPulseAudioEnabled) {

    GatewayConfiguration config;
    config[PulseGateway::ID] = "[ { \"audio\" : true } ]";
    setGatewayConfigs(config);

    CommandJob job(getLib(), "/usr/bin/paplay /usr/share/sounds/alsa/Rear_Center.wav");
    job.start();
    ASSERT_TRUE( job.isRunning() );


    ASSERT_TRUE(job.wait() == 0);

    run();
}


TEST_F(PelagicontainApp, TestStdin) {
    CommandJob job(getLib(), "/bin/cat");
    job.captureStdin();
    job.captureStdout();
    job.start();
    ASSERT_TRUE( job.isRunning() );

    const char outputBytes[] = "test string";
    char inputBytes[sizeof(outputBytes)] = {};

    auto writtenBytesCount = write( job.stdin(), outputBytes, sizeof(outputBytes) );
    ASSERT_EQ( writtenBytesCount, sizeof(outputBytes) );

    auto readBytesCount = read( job.stdout(), inputBytes, sizeof(inputBytes) );
    ASSERT_EQ( readBytesCount, sizeof(outputBytes) );

    SignalConnectionsHandler connections;
    addProcessListener( connections, job.pid(), [&] (
                int pid, int exitCode) {
                log_debug() << "finished process :" << job.toString();
                exit();
            }, getMainContext() );

    kill(job.pid(), SIGTERM);

    run();
}


/**
 * We do not enable the network gateway so we expect the ping to fail
 */
TEST_F(PelagicontainApp, TestNetworkInternetCapabilityDisabled) {
    CommandJob job(getLib(), "ping www.google.com -c 5");
    job.start();

    ASSERT_TRUE( job.isRunning() );

    bool bNetworkAccessSucceeded = false;

    SignalConnectionsHandler connections;
    addProcessListener( connections, job.pid(), [&] (
                int pid, int exitCode) {
                bNetworkAccessSucceeded = (exitCode == 0);
                exit();
            }, getMainContext() );

    run();

    ASSERT_FALSE(bNetworkAccessSucceeded);
}

/**
 * This test checks that an external is accessible after the network gateway has been enabled to access the internet.
 */
TEST_F(PelagicontainApp, TestNetworkInternetCapabilityEnabled) {

    GatewayConfiguration config;
    config[NetworkGateway::ID] = "[ { \"internet-access\" : true, \"gateway\" : \"10.0.3.1\" } ]";
    getLib().getPelagicontain().setGatewayConfigs(config);

    CommandJob job2(getLib(), "ping 8.8.8.8 -c 5");
    job2.start();

    CommandJob job(getLib(), "ping www.google.com -c 5");
    job.start();

    ASSERT_TRUE( job.isRunning() );

    bool bNetworkAccessSucceeded = false;

    SignalConnectionsHandler connections;
    addProcessListener( connections, job.pid(), [&] (
                int pid, int exitCode) {
                bNetworkAccessSucceeded = (exitCode == 0);
                exit();
            }, getMainContext() );

    run();

    ASSERT_TRUE(bNetworkAccessSucceeded);
}


//TEST(PelagicontainLib, TestStartUnexistingApp) {
//	PelagicontainApp app;
//	PelagicontainLib& lib = app.lib;
//	Job job(lib, "/usr/bin/notexisting");
//	job.start();
//	// TODO : implement
//}


TEST_F(PelagicontainApp, TestJobReturnCode) {

    CommandJob jobTrue(getLib(), "/bin/true");
    jobTrue.start();
    ASSERT_TRUE(jobTrue.wait() == 0);

    CommandJob jobFalse(getLib(), "/bin/false");
    jobFalse.start();
    ASSERT_FALSE(jobFalse.wait() == 0);

    //	addProcessListener(jobFalse.pid(), [&] (
    //			int pid, int exitCode) {
    ////		log_error () << exitCode;
    //		app.exit();
    //	}, app.getMainContext());
    //
    //	app.run();

}

/**
 * Checks that DBUS daemons are accessible if the corresponding capability is enabled
 */
TEST_F(PelagicontainApp, TestDBusGatewayWithAccess) {

    {
        GatewayConfiguration config;
        config[DBusGateway::ID] = "[{"
                "\"dbus-gateway-config-session\": [ {            \"direction\": \"*\",            \"interface\": \"*\",            \"object-path\": \"*\",            \"method\": \"*\"        }], "
                "\"dbus-gateway-config-system\": [{            \"direction\": \"*\",            \"interface\": \"*\",            \"object-path\": \"*\",            \"method\": \"*\"        }]}]";

        getLib().setGatewayConfigs(config);

        CommandJob jobTrue(
                getLib(),
                "/usr/bin/dbus-send --session --print-reply --dest=org.freedesktop.DBus / org.freedesktop.DBus.Introspectable.Introspect");
        jobTrue.start();

        ASSERT_TRUE(jobTrue.wait() == 0);
    }

    {
        CommandJob jobTrue(
                getLib(),
                "/usr/bin/dbus-send --system --print-reply --dest=org.freedesktop.DBus / org.freedesktop.DBus.Introspectable.Introspect");
        jobTrue.start();

        ASSERT_TRUE(jobTrue.wait() == 0);
    }

}



/**
 * Checks that DBUS is not accessible if the corresponding capability is not enabled
 */
TEST_F(PelagicontainApp, TestDBusGatewayWithoutAccess) {

    {
        CommandJob jobTrue(
                getLib(),
                "/usr/bin/dbus-send --session --print-reply --dest=org.freedesktop.DBus / org.freedesktop.DBus.Introspectable.Introspect");
        jobTrue.start();

        ASSERT_TRUE(jobTrue.wait() != 0);
    }

    {
        CommandJob jobTrue(
                getLib(),
                "/usr/bin/dbus-send --system --print-reply --dest=org.freedesktop.DBus / org.freedesktop.DBus.Introspectable.Introspect");
        jobTrue.start();

        // We expect the system bus to be accessible, even if we can not access any service. TODO : test if the services are accessible
        ASSERT_TRUE(jobTrue.wait() != 0);
    }

}


TEST_F(PelagicontainApp, InitTest) {
    ASSERT_TRUE( getLib().isInitialized() );
}
