/*
 *   Copyright (C) 2014 Pelagicore AB
 *   All rights reserved.
 */
#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "pelagicontain.h"
#include "pamabstractinterface.h"
#include "debug.h"
#include "log_console.h"

LOG_DEFINE_APP_IDS("PCON", "Pelagicontain");
LOG_DECLARE_CONTEXT(Pelagicontain_DefaultLogContext, "PCON", "Main context");

using namespace pelagicore;

/* Mock the PAMAbstractInterface class */
class MockPAMAbstractInterface :
	public PAMAbstractInterface
{
public:
	MOCK_METHOD2(registerClient,
		void(const std::string &cookie, const std::string &appId));

	MOCK_METHOD1(unregisterClient,
		void(const std::string &appId));

	MOCK_METHOD1(updateFinished,
		void(const std::string &appId));
};

using ::testing::InSequence;

/*! Test Pelagicontains interaction with Platform Access Manager
 *
 * The test uses a mock of the abstract interface of PAM and calls
 * metods on Pelagicontain in the same sequence as the Launcher and
 * PAM would do. The assertions are only on how Pelagicontain calls
 * PAM in response to thses calls.
 */
TEST(PelagicontainTest, TestInteractionWithPAM) {
	std::string appId = "the-app-id";

	MockPAMAbstractInterface pam;
	Pelagicontain pc(&pam);

	/* The calls should be made in the specific order as below: */
	{
		InSequence sequence;
		EXPECT_CALL(pam, registerClient("", appId)).Times(1);
		EXPECT_CALL(pam, updateFinished(appId)).Times(1);
		EXPECT_CALL(pam, unregisterClient(appId)).Times(1);
	}

	pc.launch(appId);

	pc.update(std::map<std::string, std::string>({{"", ""}}));

	pc.shutdown();
}

/* Pelagicontain raises a SIGINT signal when it has completed the shutdown
 * phase. We need to register a handle for that signal so we can ignore it.
 */
void myHandler(int s)
{
}

int main(int argc, char **argv) {
	struct sigaction sigIntHandler;
	sigIntHandler.sa_handler = myHandler;
	sigemptyset(&sigIntHandler.sa_mask);
	sigIntHandler.sa_flags = 0;
	sigaction(SIGINT, &sigIntHandler, NULL);

	ConsoleLogOutput logOuput("/dev/null");
	ConsoleLogOutput::setInstance(logOuput);

	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
