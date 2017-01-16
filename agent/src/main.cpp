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


#include <glibmm/optioncontext.h>

#include "config/config.h"
#include "config/configerror.h"
#include "config/configloader.h"
#include "config/fileconfigloader.h"
#include "config/commandlineconfigsource.h"
#include "config/mainconfigsource.h"
#include "config/defaultconfigsource.h"
#include "config/configsource.h"
#include "config/configitem.h"
#include "config/configdefinition.h"

#include "softwarecontaineragentadaptor.h"

namespace softwarecontainer {

LOG_DEFINE_APP_IDS("SCAG", "SoftwareContainer agent");
LOG_DECLARE_DEFAULT_CONTEXT(PAM_DefaultLogContext, "MAIN", "Main context");

} // namespace softwarecontainer

using namespace softwarecontainer;

/*
 * When we get signals that can be trapped, we want the main loop to be quitted.
 */
int signalHandler(void *data) {
    log_debug() << "Caught signal, exiting!";
    Glib::RefPtr<Glib::MainLoop> *ml = static_cast<Glib::RefPtr<Glib::MainLoop> *>(data);
    (*ml)->quit();
    return 0;
}


int main(int argc, char **argv)
{
    // Config file path default 'SC_CONFIG_FILE' shuld be set by the build system
    Glib::OptionEntry configOpt;
    configOpt.set_long_name("config");
    configOpt.set_short_name('c');
    configOpt.set_arg_description("<path>");
    configOpt.set_description("Path to SoftwareContainer configuration file,"
                              " defaults to \"" + std::string(SC_CONFIG_FILE) + "\"");

    Glib::OptionEntry preloadOpt;
    preloadOpt.set_long_name("preload");
    preloadOpt.set_short_name('p');
    preloadOpt.set_arg_description("<number>");
    preloadOpt.set_description("Number of containers to preload, "
                               "defaults to " + std::to_string(SC_PRELOAD_COUNT));

    Glib::OptionEntry userOpt;
    userOpt.set_long_name("user");
    userOpt.set_short_name('u');
    userOpt.set_arg_description("<uid>");
    userOpt.set_description("Default user id to be used when starting processes in the container,"
                            " defaults to 0");

    Glib::OptionEntry keepContainersAliveOpt;
    keepContainersAliveOpt.set_long_name("keep-containers-alive");
    keepContainersAliveOpt.set_short_name('k');
    keepContainersAliveOpt.set_description("Containers will not be shut down on exit. Useful for "
                                           "debugging, defaults to "
                                           + std::string(SC_KEEP_CONTAINERS_ALIVE ? "true" : "false"));

    Glib::OptionEntry timeoutOpt;
    timeoutOpt.set_long_name("timeout");
    timeoutOpt.set_short_name('t');
    timeoutOpt.set_arg_description("<seconds>");
    timeoutOpt.set_description("Timeout in seconds to wait for containers to shutdown,"
                               " defaults to " + std::to_string(SC_SHUTDOWN_TIMEOUT));

    Glib::OptionEntry serviceManifestDirOpt;
    serviceManifestDirOpt.set_long_name("manifest-dir");
    serviceManifestDirOpt.set_short_name('m');
    serviceManifestDirOpt.set_arg_description("<filepath>");
    serviceManifestDirOpt.set_description("Path to a file or directory where service manifest(s) "
                                          "exist, defaults to \""
                                          + std::string(SC_SERVICE_MANIFEST_DIR) + "\"");

    Glib::OptionEntry defaultServiceManifestDirOpt;
    defaultServiceManifestDirOpt.set_long_name("default-manifest-dir");
    defaultServiceManifestDirOpt.set_short_name('d');
    defaultServiceManifestDirOpt.set_arg_description("<filepath>");
    defaultServiceManifestDirOpt.set_description("Path to a file or directory where default "
                                                 "service manifest(s) exist, defaults to \""
                                                  + std::string(SC_DEFAULT_SERVICE_MANIFEST_DIR) + "\"");

    Glib::OptionEntry sessionBusOpt;
    sessionBusOpt.set_long_name("session-bus");
    sessionBusOpt.set_short_name('b');
    sessionBusOpt.set_description("Use the session bus instead of the system bus, "
                                  "defaults to " + std::string(SC_USE_SESSION_BUS ? "true" : "false"));


    /* Default values need to be somehting that should not be set explicitly
     * by the user. */
    Glib::ustring configPath = ConfigDefinition::SC_CONFIG_PATH_INITIAL_VALUE;
    int preloadCount = ConfigDefinition::PRELOAD_COUNT_INITIAL_VALUE;
    int userID = 0;
    bool keepContainersAlive = ConfigDefinition::KEEP_CONTAINERS_ALIVE_INITIAL_VALUE;
    int timeout = ConfigDefinition::SHUTDOWN_TIMEOUT_INITIAL_VALUE;
    Glib::ustring serviceManifestDir = ConfigDefinition::SERVICE_MANIFEST_DIR_INITIAL_VALUE;
    Glib::ustring defaultServiceManifestDir = ConfigDefinition::DEFAULT_SERVICE_MANIFEST_DIR_INITIAL_VALUE;
    bool useSessionBus = ConfigDefinition::USE_SESSION_BUS_INITIAL_VALUE;

    Glib::OptionGroup mainGroup("Options", "Options for SoftwareContainer");
    mainGroup.add_entry(configOpt, configPath);
    mainGroup.add_entry(preloadOpt, preloadCount);
    mainGroup.add_entry(userOpt, userID);
    mainGroup.add_entry(keepContainersAliveOpt, keepContainersAlive);

    mainGroup.add_entry(timeoutOpt, timeout);
    mainGroup.add_entry(serviceManifestDirOpt, serviceManifestDir);
    mainGroup.add_entry(defaultServiceManifestDirOpt, defaultServiceManifestDir);
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

    Glib::init();
    Gio::init();

    Glib::RefPtr<Glib::MainContext> mainContext = Glib::MainContext::get_default();
    Glib::RefPtr<Glib::MainLoop> ml = Glib::MainLoop::create(mainContext);

    // Config file set on commandline should take precedence over default
    std::string configFileLocation;
    if (configPath != ConfigDefinition::SC_CONFIG_PATH_INITIAL_VALUE) {
        configFileLocation = std::string(configPath);
    } else {
        configFileLocation = std::string(SC_CONFIG_FILE /* defined by CMake*/);
    }

    /*
     * Put all commandline options in lists to be passed to the commandline config source
     */
    std::vector<StringConfig> stringConfigs = std::vector<StringConfig>();
    if (serviceManifestDir != ConfigDefinition::SERVICE_MANIFEST_DIR_INITIAL_VALUE) {
        StringConfig config(ConfigDefinition::SC_GROUP,
                            ConfigDefinition::SC_SERVICE_MANIFEST_DIR_KEY,
                            serviceManifestDir);
        stringConfigs.push_back(config);
    }
    if (defaultServiceManifestDir != ConfigDefinition::DEFAULT_SERVICE_MANIFEST_DIR_INITIAL_VALUE) {
        StringConfig config(ConfigDefinition::SC_GROUP,
                            ConfigDefinition::SC_DEFAULT_SERVICE_MANIFEST_DIR_KEY,
                            defaultServiceManifestDir);
        stringConfigs.push_back(config);
    }

    std::vector<IntConfig> intConfigs = std::vector<IntConfig>();
    if (preloadCount != ConfigDefinition::PRELOAD_COUNT_INITIAL_VALUE) {
        IntConfig config(ConfigDefinition::SC_GROUP,
                         ConfigDefinition::SC_PRELOAD_COUNT_KEY,
                         preloadCount);
        intConfigs.push_back(config);
    }
    if (timeout != ConfigDefinition::SHUTDOWN_TIMEOUT_INITIAL_VALUE) {
        IntConfig config(ConfigDefinition::SC_GROUP,
                         ConfigDefinition::SC_SHUTDOWN_TIMEOUT_KEY,
                         timeout);
           intConfigs.push_back(config);
    }

    std::vector<BoolConfig> boolConfigs = std::vector<BoolConfig>();
    /* The bool options should be on/off flags so if they were set they are 'true'
     * otherwise they are still the default 'false' set above. */
    if (keepContainersAlive == true) {
        BoolConfig config(ConfigDefinition::SC_GROUP,
                          ConfigDefinition::SC_KEEP_CONTAINERS_ALIVE_KEY,
                          keepContainersAlive);
        boolConfigs.push_back(config);
    }
    if (useSessionBus == true) {
        BoolConfig config(ConfigDefinition::SC_GROUP,
                          ConfigDefinition::SC_USE_SESSION_BUS_KEY,
                          useSessionBus);
        boolConfigs.push_back(config);
    }

    try {
        // Create a config source representing the commandline options
        std::unique_ptr<ConfigSource> cmdlineConfigs(new CommandlineConfigSource(stringConfigs,
                                                                                 intConfigs,
                                                                                 boolConfigs));

        // Create a config source representing the main config file
        std::unique_ptr<ConfigLoader> loader(new FileConfigLoader(configFileLocation));
        std::unique_ptr<ConfigSource> mainConfigs(new MainConfigSource(std::move(loader),
                                                                       ConfigDefinition::typeMap()));

        // Create a config source representing the default values
        std::unique_ptr<ConfigSource> defaultConfigs(new DefaultConfigSource());

        std::vector<std::unique_ptr<ConfigSource>> configSources;
        configSources.push_back(std::move(cmdlineConfigs));
        configSources.push_back(std::move(mainConfigs));
        configSources.push_back(std::move(defaultConfigs));

        std::shared_ptr<Config> config = std::make_shared<Config>(std::move(configSources),
                                                                  ConfigDefinition::mandatory(),
                                                                  ConfigDependencies());

        ::softwarecontainer::SoftwareContainerAgent agent(mainContext, config);
        std::unique_ptr<SoftwareContainerAgentAdaptor> adaptor(new SoftwareContainerAgentAdaptor(agent, useSessionBus));

        // Register UNIX signal handler
        g_unix_signal_add(SIGINT, &signalHandler, &ml);
        g_unix_signal_add(SIGTERM, &signalHandler, &ml);
        ml->run();

        log_debug() << "Exiting softwarecontainer agent";
        return 0;
    } catch (ConfigError &error) {
        log_error() << "Could not load configuration";
        return 1;
    } catch (ReturnCode failure) {
        log_error() << "Agent initialization failed";
        return 1;
    }
}
