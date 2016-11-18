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

#include "softwarecontaineragentadaptor.h"

LOG_DEFINE_APP_IDS("SCAG", "SoftwareContainer agent");

namespace softwarecontainer {
    LOG_DECLARE_DEFAULT_CONTEXT(PAM_DefaultLogContext, "MAIN", "Main context");
}

void usage(const char *argv0)
{
    printf("SoftwareContainer agent, v.%s\n", PACKAGE_VERSION);
    printf("Usage: %s [options]\n", argv0);
    printf("Options:\n");
    printf("  -p, --preload <num>     : Number of containers to preload, defaults to 0\n");
    printf("  -u, --user <uid>        : Default user id to be used when starting processes in the container, defaults to 0\n");
    printf("  -s, --shutdown <bool>   : If false, containers will not be shutdown on exit. Useful for debugging. Defaults to true\n");
    printf("  -t, --timeout <seconds> : Timeout in seconds to wait for containers to shutdown, defaults to 1\n");
    printf("  -m, --manifest <path>   : Path to a file or directory where service manifest(s) exist, defaults to \"\"\n");
    printf("  -b, --session-bus       : Use the session bus instead of the system bus\n");
    printf("  -h, --help              : Prints this help message and exits.\n");
}

/*
 * When we get signals that can be trapped, we want the main loop to be quitted.
 */
int signalHandler(void *data) {
    log_debug() << "Caught signal, exiting!";
    Glib::RefPtr<Glib::MainLoop> *ml = static_cast<Glib::RefPtr<Glib::MainLoop> *>(data);
    (*ml)->quit();
    return 0;
}

/*
 * Tries to register the agent on the given bus. If the system bus is given and fails,
 * this tries to register to the session bus instead.
 *
 * Throws a ReturnCode on failure
 */
DBus::Connection getBusConnection(bool useSystemBus)
{
    std::string busStr = useSystemBus ? "system" : "session";
    try {
        DBus::Connection bus = useSystemBus ? DBus::Connection::SystemBus()
                                            : DBus::Connection::SessionBus();

        // Check if the name is available or not
        bool alreadyTaken = bus.has_name(AGENT_BUS_NAME);
        if (alreadyTaken) {
            log_error() << AGENT_BUS_NAME << " is already taken on the " << busStr << " bus.";
            throw ReturnCode::FAILURE;
        }

        DBus::Connection connection(bus);
        connection.request_name(AGENT_BUS_NAME);
        return connection;

    } catch (DBus::Error &err) {
        log_warning() << "Could not register " << AGENT_BUS_NAME << " to the " << busStr << " bus";
        throw ReturnCode::FAILURE;
    }
}


int main(int argc, char **argv)
{
    static struct option long_options[] =
    {
        { "preload",     required_argument, 0, 'p' },
        { "user",        required_argument, 0, 'u' },
        { "shutdown",    required_argument, 0, 's' },
        { "timeout",     required_argument, 0, 't' },
        { "manifest",    required_argument, 0, 'm' },
        { "session-bus", no_argument,       0, 'b' },
        { "help",        no_argument,       0, 'h' },
        { 0, 0, 0, 0 }
    };

    int preloadCount = 0;
    int userID = 0;
    bool shutdownContainers = true;
    int timeout = 1;
    std::string servicemanifest = "";
    bool useSystemBus = true;

    int option_index = 0;
    int c = 0;
    while((c = getopt_long(argc, argv, "p:u:s:t:m:hb", long_options, &option_index)) != -1) {
        switch(c) {
            case 'p':
                if (!parseInt(optarg, &preloadCount)) {
                    usage(argv[0]);
                    exit(1);
                }
                break;
            case 'u':
                if (!parseInt(optarg, &userID)) {
                    usage(argv[0]);
                    exit(1);
                }
                break;
            case 's':
                shutdownContainers = std::string(optarg).compare("true") == 0;
                break;
            case 't':
                if (!parseInt(optarg, &timeout)) {
                    usage(argv[0]);
                    exit(1);
                }
                break;
            case 'm':
                servicemanifest = std::string(optarg);
                break;
            case 'b':
                useSystemBus = false;
                break;
            case 'h':
                usage(argv[0]);
                exit(0);
                break;

            case '?':
                usage(argv[0]);
                exit(1);
                break;
        }
    }

    profilepoint("softwareContainerStart");
    log_debug() << "Starting softwarecontainer agent";

    Glib::RefPtr<Glib::MainContext> mainContext = Glib::MainContext::get_default();
    Glib::RefPtr<Glib::MainLoop> ml = Glib::MainLoop::create(mainContext);

    DBus::Glib::BusDispatcher dbusDispatcher;
    DBus::default_dispatcher = &dbusDispatcher;
    dbusDispatcher.attach(mainContext->gobj());

    try {
        DBus::Connection connection = getBusConnection(useSystemBus);
        SoftwareContainerAgent agent(mainContext,
                                     preloadCount,
                                     shutdownContainers,
                                     timeout,
                                     servicemanifest);
        std::unique_ptr<SoftwareContainerAgentAdaptor> adaptor =
            std::unique_ptr<SoftwareContainerAgentAdaptor>(
                new DBusCppAdaptor(connection, AGENT_OBJECT_PATH, agent)
            );

        // Register UNIX signal handler
        g_unix_signal_add(SIGINT, &signalHandler, &ml);
        g_unix_signal_add(SIGTERM, &signalHandler, &ml);
        ml->run();

        log_debug() << "Exiting softwarecontainer agent";
        return 0;
    } catch (ReturnCode failure) {
        log_error() << "Agent initialization failed";
        return 1;
    }

}
