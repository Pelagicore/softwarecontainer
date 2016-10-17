#include "softwarecontaineragentadaptor.h"

LOG_DEFINE_APP_IDS("SCAG", "SoftwareContainer agent");

namespace softwarecontainer {
    LOG_DECLARE_DEFAULT_CONTEXT(PAM_DefaultLogContext, "MAIN", "Main context");
}

void usage(const char *argv0)
{
    printf("SoftwareContainer agent, v.%s\n", PACKAGE_VERSION);
    printf("Usage: %s [-p or --preload <num>] [-u or --user <uid>]", argv0);
    printf("[-s or --shutdown <bool>] [-t or --timeout <seconds>]\n");
    printf("\n");
    printf("--preload <num>     : Number of containers to preload, defaults to 0\n");
    printf("--user <uid>        : Default user id to be used when starting processes in the container, defaults to 0\n");
    printf("--shutdown <bool>   : If false, containers will not be shutdown on exit. Useful for debugging. Defaults to true\n");
    printf("--timeout <seconds> : Timeout in seconds to wait for containers to shutdown, defaults to 2\n");
}


int signalHandler(void *data) {
    log_debug() << "Caught signal, exiting!";
    Glib::RefPtr<Glib::MainLoop> *ml = static_cast<Glib::RefPtr<Glib::MainLoop> *>(data);
    (*ml)->quit();
    return 0;
}


int main(int argc, char **argv)
{
    static struct option long_options[] =
    {
        { "preload",  required_argument, 0, 'p' },
        { "user",     required_argument, 0, 'u' },
        { "shutdown", required_argument, 0, 's' },
        { "timeout",  required_argument, 0, 't' },
        { "help",     no_argument,       0, 'h' },
        { 0, 0, 0, 0 }
    };

    int preloadCount = 0;
    int userID = 0;
    bool shutdownContainers = true;
    int timeout = 2;

    int option_index = 0;
    int c = 0;
    while((c = getopt_long(argc, argv, "p:u:s:t:",long_options, &option_index)) != -1) {
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

    std::unique_ptr<DBus::Connection> connection =
        std::unique_ptr<DBus::Connection>(new DBus::Connection(DBus::Connection::SystemBus()));

    // We try to use the system bus, and fallback to the session bus if the system bus can not be used
    try {
        connection->request_name(AGENT_BUS_NAME);
    } catch (DBus::Error &error) {
        log_warning() << "Can't own the name" << AGENT_BUS_NAME << " on the system bus => use session bus instead";
        connection = std::unique_ptr<DBus::Connection>(new DBus::Connection(DBus::Connection::SessionBus()));
        connection->request_name(AGENT_BUS_NAME);
    }

    try {
        SoftwareContainerAgent agent(mainContext, preloadCount, shutdownContainers, timeout);
        std::unique_ptr<SoftwareContainerAgentAdaptor> adaptor =
            std::unique_ptr<SoftwareContainerAgentAdaptor>(new DBusCppAdaptor(*connection, AGENT_OBJECT_PATH, agent));

    } catch (ReturnCode failure) {
        log_error() << "Agent initialization failed";
        return 1;
    }

    // Register UNIX signal handler
    g_unix_signal_add(SIGINT, &signalHandler, &ml);
    g_unix_signal_add(SIGTERM, &signalHandler, &ml);
    ml->run();

    log_debug() << "Exiting softwarecontainer agent";

    return 0;
}
