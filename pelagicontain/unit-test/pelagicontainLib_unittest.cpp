/*
 *   Copyright (C) 2014 Pelagicore AB
 *   All rights reserved.
 */

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "generators.h"
#include "pelagicontain-lib.h"

#include "dbusgateway.h"



LOG_DECLARE_DEFAULT_CONTEXT(defaultContext, "ff", "dd");

struct PelagicontainApp {

	PelagicontainApp() : lib(m_context) {
		lib.init();
	}

	void run() {
		m_ml = Glib::MainLoop::create(m_context);
		m_ml->run();
	}

	void exit() {
		m_ml->quit();
	}

	Glib::RefPtr<Glib::MainContext> getMainContext() {
		return m_context;
	}

	Glib::RefPtr<Glib::MainContext> m_context = Glib::MainContext::get_default();
	Glib::RefPtr<Glib::MainLoop> m_ml;
	PelagicontainLib lib;
};


TEST(PelagicontainLib, TestStdin) {
	PelagicontainApp app;
	PelagicontainLib& lib = app.lib;
	CommandJob job(lib, "/bin/cat");
	job.captureStdin();
	job.captureStdout();
	job.start();
	ASSERT_TRUE(job.isRunning());

	const char outputBytes[] = "test string";
	char inputBytes[sizeof(outputBytes)] = {};

	auto writtenBytesCount = write(job.stdin(), outputBytes, sizeof(outputBytes));
	ASSERT_EQ(writtenBytesCount, sizeof(outputBytes));

	auto readBytesCount = read(job.stdout(), inputBytes, sizeof(inputBytes));
	ASSERT_EQ(readBytesCount, sizeof(outputBytes));

	addProcessListener(job.pid(), [&] (
			int pid, int exitCode) {
		log_debug() << "finished process :" << job.toString();
		app.exit();
	}, app.getMainContext());

	kill(job.pid(), SIGTERM);

	app.run();
}


TEST(PelagicontainLib, TestNetworkInternetCapability) {
	PelagicontainApp app;
	PelagicontainLib& lib = app.lib;
	CommandJob job(lib, "/bin/ping www.google.com -c 5");
	job.start();

	ASSERT_TRUE(job.isRunning());

	bool bNetworkAccessSucceeded = false;
	addProcessListener(job.pid(), [&] (
			int pid, int exitCode) {
		bNetworkAccessSucceeded = (exitCode == 0);
		app.exit();
	}, app.getMainContext());

	app.run();

	ASSERT_FALSE(bNetworkAccessSucceeded);
}


//TEST(PelagicontainLib, TestStartUnexistingApp) {
//	PelagicontainApp app;
//	PelagicontainLib& lib = app.lib;
//	Job job(lib, "/usr/bin/notexisting");
//	job.start();
//	// TODO : implement
//}


TEST(PelagicontainLib, TestJobReturnCode) {
	PelagicontainApp app;
	PelagicontainLib& lib = app.lib;

	CommandJob jobTrue(lib, "/bin/true");
	jobTrue.start();
	ASSERT_TRUE(jobTrue.wait() == 0);

	CommandJob jobFalse(lib, "/bin/false");
	jobFalse.start();
	ASSERT_FALSE(jobTrue.wait() == 0);

//	addProcessListener(jobFalse.pid(), [&] (
//			int pid, int exitCode) {
////		log_error () << exitCode;
//		app.exit();
//	}, app.getMainContext());
//
//	app.run();

	ASSERT_TRUE(jobFalse.wait() != 0);
}

TEST(PelagicontainLib, TestDBusGatewayWithAccess) {
	PelagicontainApp app;
	PelagicontainLib& lib = app.lib;

	{
		GatewayConfiguration config;
		config[DBusGateway::ID] = "{"
				"\"dbus-gateway-config-session\": [ {            \"direction\": \"*\",            \"interface\": \"*\",            \"object-path\": \"*\",            \"method\": \"*\"        }], "
				"\"dbus-gateway-config-system\": [{            \"direction\": \"*\",            \"interface\": \"*\",            \"object-path\": \"*\",            \"method\": \"*\"        }]}";

		lib.getPelagicontain().update(config);

		CommandJob jobTrue(lib,
				"/usr/bin/dbus-send --session --print-reply --dest=org.freedesktop.DBus / org.freedesktop.DBus.Introspectable.Introspect");
		jobTrue.start();

		ASSERT_TRUE(jobTrue.wait() == 0);
	}

	{
		CommandJob jobTrue(lib,
				"/usr/bin/dbus-send --system --print-reply --dest=org.freedesktop.DBus / org.freedesktop.DBus.Introspectable.Introspect");
		jobTrue.start();

		ASSERT_TRUE(jobTrue.wait() == 0);
	}

}


TEST(PelagicontainLib, TestDBusGatewayWithoutAccess) {
	PelagicontainApp app;
	PelagicontainLib& lib = app.lib;

	{
		CommandJob jobTrue(lib,
				"/usr/bin/dbus-send --session --print-reply --dest=org.freedesktop.DBus / org.freedesktop.DBus.Introspectable.Introspect");
		jobTrue.start();

		ASSERT_TRUE(jobTrue.wait() != 0);
	}

	{
		CommandJob jobTrue(lib,
				"/usr/bin/dbus-send --system --print-reply --dest=org.freedesktop.DBus / org.freedesktop.DBus.Introspectable.Introspect");
		jobTrue.start();

		// We expect the system bus to be accessible, even if we can not access any service. TODO : test if the services are accessible
		ASSERT_TRUE(jobTrue.wait() != 0);
	}

}


TEST(PelagicontainLib, InitTest) {
	PelagicontainApp app;
    ASSERT_TRUE(app.lib.isInitialized());
}

