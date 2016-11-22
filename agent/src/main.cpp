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


#include <glibmm/optioncontext.h>

#include "config/config.h"
#include "config/configloaderabstractinterface.h"
#include "config/fileconfigloader.h"

#include "softwarecontaineragentadaptor.h"


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
    // Config file path default 'SC_CONFIG_FILE' shuld be set by the build system
    Glib::OptionEntry configOpt;
    configOpt.set_long_name("config");
    configOpt.set_short_name('c');
    configOpt.set_arg_description("<path>");
    configOpt.set_description("Path to SoftwareContainer configuration file, defaults to \"" + std::string(SC_CONFIG_FILE) + "\"");

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

    Glib::OptionEntry keepContainersAliveOpt;
    keepContainersAliveOpt.set_long_name("keep-containers-alive");
    keepContainersAliveOpt.set_short_name('k');
    keepContainersAliveOpt.set_description("Containers will not be shut down on exit. Useful for debugging");

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


    /* Default values need to be somehting that should not be set explicitly
     * by the user.
     */
    Glib::ustring configPath = "";
    int preloadCount = -1;
    int userID = 0;
    bool keepContainersAlive = false;
    int timeout = -2;
    Glib::ustring servicemanifest = "";
    bool useSessionBus = false;

    Glib::OptionGroup mainGroup("Options", "Options for SoftwareContainer");
    mainGroup.add_entry(configOpt, configPath);
    mainGroup.add_entry(preloadOpt, preloadCount);
    mainGroup.add_entry(userOpt, userID);
    mainGroup.add_entry(keepContainersAliveOpt, keepContainersAlive);

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

    // Config file set on command line should take precedence over default
    std::string configFileLocation;
    if (configPath != "") {
        configFileLocation = std::string(configPath);
    } else {
        configFileLocation = std::string(SC_CONFIG_FILE);
    }

    /* Put all configs explicitly set as command line options in maps so they
     * can be passed to Config, and thus enable Config to prioritize these values
     * over the static configuration.
     */
    std::map<std::string, std::string> stringOptions;
    if (servicemanifest != "") {
        stringOptions.insert(std::pair<std::string, std::string>(Config::SERVICE_MANIFEST_DIR,
                                                                 servicemanifest));
    }

    std::map<std::string, int> intOptions;
    if (preloadCount != -1) {
        intOptions.insert(std::pair<std::string, int>(Config::PRELOAD_COUNT, preloadCount));
    }
    if (timeout != -2) {
        intOptions.insert(std::pair<std::string, int>(Config::SHUTDOWN_TIMEOUT, timeout));
    }

    std::map<std::string, bool> boolOptions;
    /* The bool options should be on/off flags so if they were set they are 'true'
     * otherwise they are still the default 'false' set above.
     */
    if (keepContainersAlive == true) {
        boolOptions.insert(std::pair<std::string, bool>(Config::KEEP_ALIVE, keepContainersAlive));
    }
    if (useSessionBus == true) {
        boolOptions.insert(std::pair<std::string, bool>(Config::USE_SESSION_BUS, useSessionBus));
    }

    std::unique_ptr<ConfigLoaderAbstractInterface> loader(new FileConfigLoader(configFileLocation));
    Config config(std::move(loader), stringOptions, intOptions, boolOptions);

    try {
        DBus::Connection connection = getBusConnection(useSessionBus);
        SoftwareContainerAgent agent(mainContext, config);
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
