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
#include <glibmm/optioncontext.h>

LOG_DEFINE_APP_IDS("SCAG", "SoftwareContainer agent");

namespace softwarecontainer {
    LOG_DECLARE_DEFAULT_CONTEXT(PAM_DefaultLogContext, "MAIN", "Main context");
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
DBus::Connection getBusConnection(bool useSessionBus)
{
    std::string busStr = useSessionBus ? "session" : "system";
    try {
        DBus::Connection bus = useSessionBus ? DBus::Connection::SessionBus()
                                             : DBus::Connection::SystemBus();

        // Check if the name is available or not
        bool alreadyTaken = bus.has_name(AGENT_BUS_NAME);
        if (alreadyTaken) {
            log_error() << AGENT_BUS_NAME << " is already taken on the " << busStr << " bus.";
            throw ReturnCode::FAILURE;
        }

        DBus::Connection connection(bus);
        connection.request_name(AGENT_BUS_NAME);
        log_debug() << "Registered " << AGENT_BUS_NAME << " on the " << busStr << " bus.";
        return connection;

    } catch (DBus::Error &err) {
        log_warning() << "Could not register " << AGENT_BUS_NAME << " to the " << busStr << " bus";
        throw ReturnCode::FAILURE;
    }
}


int main(int argc, char **argv)
{
    Glib::OptionEntry preloadOpt;
    preloadOpt.set_long_name("preload");
    preloadOpt.set_short_name('p');
    preloadOpt.set_arg_description("<number>");
    preloadOpt.set_description("Number of containers to preload, defaults to 0");

    Glib::OptionEntry userOpt;
    userOpt.set_long_name("user");
    userOpt.set_short_name('u');
    userOpt.set_arg_description("<uid>");
    userOpt.set_description("Default user id to be used when starting processes in the container, defaults to 0");

    Glib::OptionEntry shutdownOpt;
    shutdownOpt.set_long_name("shutdown");
    shutdownOpt.set_short_name('s');
    shutdownOpt.set_arg_description("<bool>");
    shutdownOpt.set_description("If false, containers will not be shutdown on exit. Useful for debugging. Defaults to true");

    Glib::OptionEntry timeoutOpt;
    timeoutOpt.set_long_name("timeout");
    timeoutOpt.set_short_name('t');
    timeoutOpt.set_arg_description("<seconds>");
    timeoutOpt.set_description("Timeout in seconds to wait for containers to shutdown, defaults to 1");

    Glib::OptionEntry manifestOpt;
    manifestOpt.set_long_name("manifest");
    manifestOpt.set_short_name('m');
    manifestOpt.set_arg_description("<filepath>");
    manifestOpt.set_description("Path to a file or directory where service manifest(s) exist, defaults to nothing");

    Glib::OptionEntry sessionBusOpt;
    sessionBusOpt.set_long_name("session-bus");
    sessionBusOpt.set_short_name('b');
    sessionBusOpt.set_description("Use the session bus instead of the system bus");

    int preloadCount = 0;
    int userID = 0;
    bool shutdownContainers = true;
    int timeout = 1;
    Glib::ustring servicemanifest = "";
    bool useSessionBus = false;

    Glib::OptionGroup mainGroup("Options", "Options for SoftwareContainer");
    mainGroup.add_entry(preloadOpt, preloadCount);
    mainGroup.add_entry(userOpt, userID);
    mainGroup.add_entry(shutdownOpt, shutdownContainers);
    mainGroup.add_entry(timeoutOpt, timeout);
    mainGroup.add_entry(manifestOpt, servicemanifest);
    mainGroup.add_entry(sessionBusOpt, useSessionBus);

    Glib::OptionContext optionContext;
    optionContext.set_help_enabled(true); // Automatic --help
    optionContext.set_ignore_unknown_options(false);
    optionContext.set_summary("SoftwareContainer Agent v" + std::string(PACKAGE_VERSION));
    optionContext.set_description("Documentation is available at http://pelagicore.github.io/softwarecontainer");
    optionContext.set_main_group(mainGroup);

    try {
        optionContext.parse(argc, argv);
    } catch (Glib::OptionError &err) {
        log_error() << err.what();
        exit(1);
    }

    profilepoint("softwareContainerStart");
    log_debug() << "Starting softwarecontainer agent";

    Glib::RefPtr<Glib::MainContext> mainContext = Glib::MainContext::get_default();
    Glib::RefPtr<Glib::MainLoop> ml = Glib::MainLoop::create(mainContext);

    DBus::Glib::BusDispatcher dbusDispatcher;
    DBus::default_dispatcher = &dbusDispatcher;
    dbusDispatcher.attach(mainContext->gobj());

    try {
        DBus::Connection connection = getBusConnection(useSessionBus);
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
