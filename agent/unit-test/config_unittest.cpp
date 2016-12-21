
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

#include "softwarecontainer-common.h"
#include "config/config.h"
#include "config/fileconfigloader.h"
#include "config/mandatoryconfigs.h"

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include <string>
#include <glibmm.h>


/*
 * Test stub - StringConfigLoader
 *
 * Loads a Glib::KeyFile config from a string, compared to the "real" loader which reads
 * from file.
 */
class StringConfigLoader : public ConfigLoaderAbstractInterface
{

LOG_DECLARE_CLASS_CONTEXT("CFGL", "SoftwareContainer general config loader");

public:
    // Constructor just needs to init parent with the config source string
    StringConfigLoader(const std::string &source) : ConfigLoaderAbstractInterface(source) {}

    std::unique_ptr<Glib::KeyFile> loadConfig() override
    {
        std::unique_ptr<Glib::KeyFile> configData = std::unique_ptr<Glib::KeyFile>(new Glib::KeyFile);
        try {
            configData->load_from_data(Glib::ustring(this->m_source), Glib::KEY_FILE_NONE);
        } catch (Glib::KeyFileError &error) {
            log_error() << "Could not load SoftwareContainer config: \"" << error.what() << "\"";
            throw error;
        }

        return configData;
    }
};


/*
 * Test stub - PreparedConfigDefaults
 *
 * Used for initializing a DefaultConfigs parent with values
 * to support testing.
 */
class PreparedConfigDefaults : public ConfigDefaults
{
public:
    PreparedConfigDefaults(std::map<std::string, std::string> stringOptions,
                           std::map<std::string, int> intOptions,
                           std::map<std::string, bool> boolOptions)
    {
        m_stringOptions = stringOptions;
        m_intOptions = intOptions;
        m_boolOptions = boolOptions;
    }

    ~PreparedConfigDefaults() {}
};


/*
 * ConfigTest suite
 *
 * This suite tests the Config class. It uses a stubbed loader, the StringConfigLoader, to
 * provide the config source, and does asserts on the way Config parses the config provided.
 */
class ConfigTest : public ::testing::Test
{
public:
    /*
     * Empty defaults, can only be used if the test is never to fall back on default config values
     */
    std::unique_ptr<ConfigDefaults> emptyDefaultConfig()
    {
        return std::unique_ptr<ConfigDefaults>(
            new PreparedConfigDefaults(std::map<std::string, std::string>(),
                                       std::map<std::string, int>(),
                                       std::map<std::string, bool>()));
    }
};


/*
 * Test that default string values are used when nothing else is specified.
 *
 * The loader will provide an empty config, in which case the Config class should fall back
 * on the defaults provided.
 */
TEST_F(ConfigTest, FallsBackOnDefaultStringValues) {
    // Well formed config empty of values.
    const std::string configString = "[SoftwareContainer]\n";

    std::unique_ptr<ConfigLoaderAbstractInterface> loader(new StringConfigLoader(configString));

    std::map<std::string, std::string> stringOptions;
    stringOptions.insert(std::pair<std::string, std::string>("sc.foo", "bar"));

    std::unique_ptr<ConfigDefaults> defaults(
        new PreparedConfigDefaults(stringOptions,
                                   std::map<std::string, int>(),
                                   std::map<std::string, bool>()));

    Config config(std::move(loader), std::move(defaults));

    ASSERT_EQ(config.getStringValue("SoftwareContainer", "sc.foo"), "bar");
}

/*
 * Same as above but with integer values
 */
TEST_F(ConfigTest, FallsBackOnDefaultIntegerValues) {
    // Well formed config empty of values.
    const std::string configString = "[SoftwareContainer]\n";

    std::unique_ptr<ConfigLoaderAbstractInterface> loader(new StringConfigLoader(configString));

    std::map<std::string, int> intOptions;
    intOptions.insert(std::pair<std::string, int>("sc.foo", 123));

    std::unique_ptr<ConfigDefaults> defaults(
        new PreparedConfigDefaults(std::map<std::string, std::string>(),
                                   intOptions,
                                   std::map<std::string, bool>()));

    Config config(std::move(loader), std::move(defaults));

    ASSERT_EQ(config.getIntegerValue("SoftwareContainer", "sc.foo"), 123);
}

/*
 * Same as above but with bool values
 */
TEST_F(ConfigTest, FallsBackOnDefaultBooleanValues) {
    // Well formed config empty of values.
    const std::string configString = "[SoftwareContainer]\n";

    std::unique_ptr<ConfigLoaderAbstractInterface> loader(new StringConfigLoader(configString));

    std::map<std::string, bool> boolOptions;
    boolOptions.insert(std::pair<std::string, bool>("sc.foo", true));

    std::unique_ptr<ConfigDefaults> defaults(
        new PreparedConfigDefaults(std::map<std::string, std::string>(),
                                   std::map<std::string, int>(),
                                   boolOptions));

    Config config(std::move(loader), std::move(defaults));

    ASSERT_EQ(config.getBooleanValue("SoftwareContainer", "sc.foo"), true);
}

/*
 * Test that Config throws exception on wrong config group
 */
TEST_F(ConfigTest, IncorrectGroupThrows) {
    // Well formed config
    const std::string configString = "[SoftwareContainer]\n"
                                     "sc.foo=bar\n"
                                     "sc.preload=2\n";

    std::unique_ptr<ConfigLoaderAbstractInterface> loader(new StringConfigLoader(configString));

    Config config(std::move(loader), std::move(emptyDefaultConfig()));

    ASSERT_THROW(config.getStringValue("DoesNotExist", "sc.foo"), ConfigError);
}

/*
 * Test that Config throws exception on wrong key, when getting string value
 */
TEST_F(ConfigTest, IncorrectKeyThrowsForStringValue) {
    // Well formed config
    const std::string configString = "[SoftwareContainer]\n"
                                     "sc.foo=bar\n"
                                     "sc.preload=2\n";

    std::unique_ptr<ConfigLoaderAbstractInterface> loader(new StringConfigLoader(configString));

    Config config(std::move(loader), std::move(emptyDefaultConfig()));

    ASSERT_THROW(config.getStringValue("SoftwareContainer", "does-not-exist"), ConfigError);
}

/*
 * As above but for integer values
 */
TEST_F(ConfigTest, IncorrectKeyThrowsForIntValue) {
    // Well formed config
    const std::string configString = "[SoftwareContainer]\n"
                                     "sc.foo=bar\n"
                                     "sc.preload=2\n";

    std::unique_ptr<ConfigLoaderAbstractInterface> loader(new StringConfigLoader(configString));

    Config config(std::move(loader), std::move(emptyDefaultConfig()));

    ASSERT_THROW(config.getIntegerValue("SoftwareContainer", "does-not-exist"), ConfigError);
}

/*
 * As above but for bool values
 */
TEST_F(ConfigTest, IncorrectKeyThrowsForBoolValue) {
    // Well formed config
    const std::string configString = "[SoftwareContainer]\n"
                                     "sc.foo=bar\n"
                                     "sc.preload=2\n";

    std::unique_ptr<ConfigLoaderAbstractInterface> loader(new StringConfigLoader(configString));

    Config config(std::move(loader), std::move(emptyDefaultConfig()));

    ASSERT_THROW(config.getBooleanValue("SoftwareContainer", "does-not-exist"), ConfigError);
}

/*
 * Test that Config throws exception on badly formatted config
 */
TEST_F(ConfigTest, UnsucessfulCreationThrows) {
    // This config is broken, it's missing a bracket in the group
    const std::string configString = "[SoftwareContainer\n"
                                     "sc.foo=bar\n";

    std::unique_ptr<ConfigLoaderAbstractInterface> loader(new StringConfigLoader(configString));

    ASSERT_THROW(Config config(std::move(loader), std::move(emptyDefaultConfig())), ConfigError);
}

/*
 * Test that Config contain expected string value.
 */
TEST_F(ConfigTest, ContainExpectedStringValue) {
    // Well formed config
    const std::string configString = "[SoftwareContainer]\n"
                                     "sc.foo=bar\n"
                                     "sc.preload=2\n";

    std::unique_ptr<ConfigLoaderAbstractInterface> loader(new StringConfigLoader(configString));

    Config config(std::move(loader), std::move(emptyDefaultConfig()));

    ASSERT_TRUE(config.getStringValue("SoftwareContainer", "sc.foo") == "bar");
}

/*
 * Test that Config contain expected integer value.
 */
TEST_F(ConfigTest, ContainExpectedIntegerValue) {
    // Well formed config
    const std::string configString = "[SoftwareContainer]\n"
                                     "sc.foo=bar\n"
                                     "sc.preload=2\n";

    std::unique_ptr<ConfigLoaderAbstractInterface> loader(new StringConfigLoader(configString));

    Config config(std::move(loader), std::move(emptyDefaultConfig()));

    ASSERT_TRUE(config.getIntegerValue("SoftwareContainer", "sc.preload") == 2);
}

/*
 * Test that Config contain expected boolean value.
 */
TEST_F(ConfigTest, ContainExpectedBooleanValue) {
    // Well formed config
    const std::string configString = "[SoftwareContainer]\n"
                                     "sc.foo = bar\n"
                                     "shut-down-containers = true\n";

    std::unique_ptr<ConfigLoaderAbstractInterface> loader(new StringConfigLoader(configString));

    Config config(std::move(loader), std::move(emptyDefaultConfig()));

    ASSERT_TRUE(config.getBooleanValue("SoftwareContainer", "shut-down-containers") == true);
}

/*
 * Test that values passed explicitly takes precedence over values read
 * from config
 */
TEST_F(ConfigTest, ExplicitStringValuesTakesPrecedence) {
    // Well formed config
    const std::string configString = "[SoftwareContainer]\n"
                                     "sc.foo = bar\n";

    std::unique_ptr<ConfigLoaderAbstractInterface> loader(new StringConfigLoader(configString));

    std::map<std::string, std::string> stringOptions;
    stringOptions.insert(std::pair<std::string, std::string>("sc.foo", "baz"));

    Config config(std::move(loader),
                  std::move(emptyDefaultConfig()),
                  stringOptions,
                  std::map<std::string, int>(),
                  std::map<std::string, bool>());

    ASSERT_TRUE(config.getStringValue("SoftwareContainer", "sc.foo") == "baz");
}

/*
 * As above but for integer values
 */
TEST_F(ConfigTest, ExplicitIntegerValuesTakesPrecedence) {
    // Well formed config
    const std::string configString = "[SoftwareContainer]\n"
                                     "sc.preload = 2\n";

    std::unique_ptr<ConfigLoaderAbstractInterface> loader(new StringConfigLoader(configString));

    std::map<std::string, int> intOptions;
    intOptions.insert(std::pair<std::string, int>("sc.preload", 2));

    Config config(std::move(loader),
                  std::move(emptyDefaultConfig()),
                  std::map<std::string, std::string>(),
                  intOptions,
                  std::map<std::string, bool>());

    ASSERT_TRUE(config.getIntegerValue("SoftwareContainer", "sc.preload") == 2);
}

/*
 * As above but for integer values
 */
TEST_F(ConfigTest, ExplicitBoolValuesTakesPrecedence) {
    // Well formed config
    const std::string configString = "[SoftwareContainer]\n"
                                     "sc.shut-down-containers = true\n";

    std::unique_ptr<ConfigLoaderAbstractInterface> loader(new StringConfigLoader(configString));

    std::map<std::string, bool> boolOptions;
    boolOptions.insert(std::pair<std::string, bool>("sc.shut-down-containers", true));

    Config config(std::move(loader),
                  std::move(emptyDefaultConfig()),
                  std::map<std::string, std::string>(),
                  std::map<std::string, int>(),
                  boolOptions);

    ASSERT_TRUE(config.getBooleanValue("SoftwareContainer", "sc.shut-down-containers") == true);
}

/*
 * Test that Config throws an exception if a config dependency is not met
 *
 * TODO: Fix this when dependecies are implemented
 */
// TEST_F(ConfigTest, MissingDependencyThrows) {
//     // Well formed config
//     const std::string configString = "[SoftwareContainer]\n"
//                                      "optional-config = 1\n";
// 
//     std::unique_ptr<ConfigLoaderAbstractInterface> loader(new StringConfigLoader(configString));
// 
//     // Define what keys that are dependencies
//     std::vector<std::string> dependencyKeys {"dependency-1", "dependency-2"};
// 
//     // Define the dependee config key
//     std::string dependee = "my-config";
// 
//     // Associate the dependee to dependencies
//     std::pair<std::string, std::vector<std::string>> dependencyAssociation(dependee, dependencyKeys);
// 
//     // Add the association between dependee and dependencies to a list
//     std::vector<std::pair<std::string, std::vector<std::string>>> dependencies {dependencyAssociation};
// 
//     Config config(std::move(loader), std::move(emptyDefaultConfig()), dependencies);
// 
//     // We should get an exception if we ask for a config that have unmet dependencies
//     ASSERT_THROW(config.getStringValue("SoftwareContainer", dependee),
//                  ConfigDependencyError);
// }

/*
 * Test that Config throws an exception if a mandatory config group is not found in
 * the main config source. In this test it's only relevant to use the main config source
 * since that is the only source where groups are used.
 */
TEST_F(ConfigTest, MissingMandatoryGroupThrows) {
    // Well formed config without the mandatory config group (see below)
    const std::string configString = "[OptionalGroup]\n"
                                     "optional-config = 1\n";

    // Now, the "main config" source will not provide the mandatory config group
    std::unique_ptr<ConfigLoaderAbstractInterface> loader(new StringConfigLoader(configString));

    // There are no dependencies between configs
    std::vector<std::pair<std::string, std::vector<std::string>>> dependencies;

    // Set up a mandatory group-key pair, the group will now be required to exist
    MandatoryConfigs mandatory = MandatoryConfigs();
    mandatory.addConfig("SoftwareContainer", "optional-config", ConfigType::Integer);

    // Since the required 'SoftwareContainer' group is not present in any config source
    // this should now throw an exception.
    ASSERT_THROW(Config config(std::move(loader),
                               std::move(emptyDefaultConfig()),
                               mandatory,
                               dependencies),
                 ConfigMandatoryError);
}

/*
 * Test that Config throws an exception if a mandatory config is not found in the
 * main config file source, or in the command line options source.
 */
TEST_F(ConfigTest, MissingMandatoryConfigThrows) {
    // Well formed config without the mandatory config (see below)
    const std::string configString = "[OptionalGroup]\n"
                                     "optional-config = 1\n";

    // Now, the "main config" source will not provide the mandatory config
    std::unique_ptr<ConfigLoaderAbstractInterface> loader(new StringConfigLoader(configString));

    // There are no dependencies between configs
    std::vector<std::pair<std::string, std::vector<std::string>>> dependencies;

    // Set up a mandatory group-key pair, the config will now be required to exist
    MandatoryConfigs mandatory = MandatoryConfigs();
    mandatory.addConfig("OptionalGroup", "mandatory-config", ConfigType::Integer);

    // Since the required 'mandatory-config' config is not present in any config source
    // this should now throw an exception.
    ASSERT_THROW(Config config(std::move(loader),
                               std::move(emptyDefaultConfig()),
                               mandatory,
                               dependencies),
                 ConfigMandatoryError);
}

/*
 * Same as above but also passing non-empty command line options as config source as well
 */
TEST_F(ConfigTest, UsingCommandLineOptionsMissingMandatoryConfigThrows) {
    // Well formed config without the mandatory config (see below)
    const std::string configString = "[OptionalGroup]\n"
                                     "optional-config = 1\n";

    // Now, the "main config" source will not provide the mandatory config
    std::unique_ptr<ConfigLoaderAbstractInterface> loader(new StringConfigLoader(configString));

    // Now, the command line options wil not provide the mandatory config either
    std::map<std::string, int> intOptions;
    intOptions.insert(std::pair<std::string, int>("another-optional-config", 2));

    // There are no dependencies between configs
    std::vector<std::pair<std::string, std::vector<std::string>>> dependencies;

    // Set up a mandatory group-key pair, the config will now be required to exist
    MandatoryConfigs mandatory = MandatoryConfigs();
    mandatory.addConfig("OptionalGroup", "mandatory-config", ConfigType::Integer);

    // Since the required 'mandatory-config' config is not present in any config source
    // this should now throw an exception.
    ASSERT_THROW(Config config(std::move(loader),
                               std::move(emptyDefaultConfig()),
                               mandatory,
                               dependencies,
                               std::map<std::string, std::string>(),
                               intOptions,
                               std::map<std::string, bool>()),
                 ConfigMandatoryError);
}


// Tests for FileConfigLoader /////////////////////////////////////////////////////////////////////

/*
 * Test that using the real FileConfigLoader and passing a path to a non-existent
 * config file results in the expected exception FileError.
 *
 * The other possible exception from FileConfigLoader is KeyFileError when the file
 * exists but is not well formed, but that is not tested in the unit-tests becuase of the
 * dependency to having actual files.
 */
TEST_F(ConfigTest, LoadingMissingConfigFileThrows) {
    FileConfigLoader loader("non-existent-path");

    ASSERT_THROW(loader.loadConfig(), Glib::FileError);
}


// Tests for MandatoryConfigs /////////////////////////////////////////////////////////////////////

/*
 * Test that we get the expected group names after adding mandatory configs
 */
TEST_F(ConfigTest, ReturnsExpectedGroups) {
    MandatoryConfigs mandatory = MandatoryConfigs();
    mandatory.addConfig("MyGroup1", "my-config1", ConfigType::Integer);
    mandatory.addConfig("MyGroup2", "my-config2", ConfigType::Integer);
    std::vector<std::string> groups = mandatory.groups();

    std::vector<std::string> expected {"MyGroup1", "MyGroup2"};

    EXPECT_THAT(groups, ::testing::ContainerEq(expected));
}

/*
 * Test that we get the expected group-key pairs after adding mandatory configs
 */
TEST_F(ConfigTest, ReturnsExpectedConfigs) {
    MandatoryConfigs mandatory = MandatoryConfigs();
    mandatory.addConfig("MyGroup1", "my-config1", ConfigType::Integer);
    mandatory.addConfig("MyGroup2", "my-config2", ConfigType::Integer);
    std::vector<std::tuple<std::string, std::string, ConfigType>> configs = mandatory.configs();

    std::tuple<std::string, std::string, ConfigType> config1("MyGroup1",
                                                             "my-config1",
                                                             ConfigType::Integer);
    std::tuple<std::string, std::string, ConfigType> config2("MyGroup2",
                                                             "my-config2",
                                                             ConfigType::Integer);
    std::vector<std::tuple<std::string, std::string, ConfigType>> expected {config1, config2};

    EXPECT_THAT(configs, ::testing::ContainerEq(expected));
}

/*
 * Test that we get empty group values when nothing has been set
 */
TEST_F(ConfigTest, HandlesGroupWhenNothingIsSet) {
    MandatoryConfigs mandatory = MandatoryConfigs();

    std::vector<std::string> groups = mandatory.groups();

    EXPECT_THAT(groups, ::testing::IsEmpty());
}

/*
 * Test that we get empty configs when nothing has been set
 */
TEST_F(ConfigTest, HandlesConfigsWhenNothingIsSet) {
    MandatoryConfigs mandatory = MandatoryConfigs();

    std::vector<std::tuple<std::string, std::string, ConfigType>> configs = mandatory.configs();

    EXPECT_THAT(configs, ::testing::IsEmpty());
}
