
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

#include "softwarecontainer-common.h"

#include "config/config.h"
#include "config/configerror.h"
#include "config/fileconfigloader.h"
#include "config/configtypes.h"
#include "config/configitem.h"
#include "config/configsource.h"
#include "config/mainconfigsource.h"
#include "config/configloader.h"
#include "config/configdefinition.h"

#include "gtest/gtest.h"
#include "gmock/gmock.h"


using namespace softwarecontainer;

/*
 * Test stub - StringConfigLoader
 *
 * Loads a Glib::KeyFile config from a string, compared to the "real" loader which reads
 * from file.
 */
class StringConfigLoader : public ConfigLoader
{

LOG_DECLARE_CLASS_CONTEXT("CFGL", "SoftwareContainer main config loader");

public:
    // Constructor just needs to init parent with the config source string
    StringConfigLoader(const std::string &source) : ConfigLoader(source) {}

    std::unique_ptr<Glib::KeyFile> loadConfig() override
    {
        std::unique_ptr<Glib::KeyFile> configData = std::unique_ptr<Glib::KeyFile>(new Glib::KeyFile);
        try {
            configData->load_from_data(Glib::ustring(this->m_source), Glib::KEY_FILE_NONE);
        } catch (Glib::KeyFileError &error) {
            log_error() << "Could not load SoftwareContainer config: \"" << error.what() << "\"";
            throw;
        }

        return configData;
    }
};


/*
 * This test stub is a config source that tests use for adding config items of any type
 * and with any ConfigSourceType needed for the tests.
 *
 * This allows one source to be setup in the tests while still providing config items
 * with different source types set.
 */
class StubbedConfigSource : public ConfigSource
{
public:
    StubbedConfigSource():
        m_stringConfigs(std::vector<StringConfig>()),
        m_intConfigs(std::vector<IntConfig>()),
        m_boolConfigs(std::vector<BoolConfig>())
    {
    }

    std::vector<StringConfig> stringConfigs() override
    {
        return m_stringConfigs;
    }

    std::vector<IntConfig> intConfigs() override
    {
        return m_intConfigs;
    }

    std::vector<BoolConfig> boolConfigs() override
    {
        return m_boolConfigs;
    }

    void addConfig(std::string group, std::string key, std::string value, ConfigSourceType source)
    {
        StringConfig config(group, key, value);
        config.setSource(source);
        m_stringConfigs.push_back(config);
    }

    void addConfig(std::string group, std::string key, int value, ConfigSourceType source)
    {
        IntConfig config(group, key, value);
        config.setSource(source);
        m_intConfigs.push_back(config);
    }

    void addConfig(std::string group, std::string key, bool value, ConfigSourceType source)
    {
        BoolConfig config(group, key, value);
        config.setSource(source);
        m_boolConfigs.push_back(config);
    }

private:
    std::vector<StringConfig> m_stringConfigs;
    std::vector<IntConfig> m_intConfigs;
    std::vector<BoolConfig> m_boolConfigs;
};



// ConfigTest ////////////////////////////////////////////////////////////////////////////////////

class ConfigTest : public ::testing::Test
{
public:
    /*
     * Convenience method to allow calling child class method on base
     * class pointer in the tests.
     *
     * Since Config takes a vector of unique_ptr<ConfigSource> we need to store
     * the stubbed source as a pointer to that base class, but the tests need
     * to call the child-only method to add config items. This is not how this
     * is intended to be used in production code, it's just a pattern for the
     * tests.
     */
    StubbedConfigSource *cast(std::unique_ptr<ConfigSource> &base)
    {
        return static_cast<StubbedConfigSource *>(base.get());
    }
};


/*
 * Test that Config can be initialized with an empty config source, i.e. a source that
 * returns empty lists of config items.
 */
TEST_F(ConfigTest, HandlesEmptySources) {
    // Create a config source but don't add any config, it's now empty
    std::unique_ptr<ConfigSource> source(new StubbedConfigSource());

    // Put source in list so it can be passed to Config
    std::vector<std::unique_ptr<ConfigSource>> sources;
    sources.push_back(std::move(source));

    // Everything is empty
    ASSERT_NO_THROW(Config config(std::move(sources), MandatoryConfigs(), ConfigDependencies()));
}

/*
 * Test that Config returns the expected string value from a source
 */
TEST_F(ConfigTest, ReturnsExpecedStringValue) {
    // Create a config source and add a config item to it
    std::unique_ptr<ConfigSource> source(new StubbedConfigSource());
    cast(source)->addConfig("MyGroup1", "my-key1", std::string("myValue1"), ConfigSourceType::Main);

    // Put source in list so it can be passed to Config
    std::vector<std::unique_ptr<ConfigSource>> sources;
    sources.push_back(std::move(source));

    Config config(std::move(sources), MandatoryConfigs(), ConfigDependencies());

    ASSERT_EQ(config.getStringValue("MyGroup1", "my-key1"), "myValue1");
}

/*
 * Test that Config returns the expected integer value from a source
 */
TEST_F(ConfigTest, ReturnsExpecedIntValue) {
    // Create a config source and add a config item to it
    std::unique_ptr<ConfigSource> source(new StubbedConfigSource());
    cast(source)->addConfig("MyGroup1", "my-int-key1", 1, ConfigSourceType::Main);

    // Put source in list so it can be passed to Config
    std::vector<std::unique_ptr<ConfigSource>> sources;
    sources.push_back(std::move(source));

    Config config(std::move(sources), MandatoryConfigs(), ConfigDependencies());

    ASSERT_EQ(config.getIntValue("MyGroup1", "my-int-key1"), 1);
}

/*
 * Test that Config returns the expected boolean value from a source
 */
TEST_F(ConfigTest, ReturnsExpecedBoolValue) {
    // Create a config source and add a config item to it
    std::unique_ptr<ConfigSource> source(new StubbedConfigSource());
    cast(source)->addConfig("MyGroup1", "my-bool-key1", true, ConfigSourceType::Main);

    // Put source in list so it can be passed to Config
    std::vector<std::unique_ptr<ConfigSource>> sources;
    sources.push_back(std::move(source));

    Config config(std::move(sources), MandatoryConfigs(), ConfigDependencies());

    ASSERT_EQ(config.getBoolValue("MyGroup1", "my-bool-key1"), true);
}

/*
 * Test that calling getters for non existing configs restults in the expected exception
 *
 * This test is for strings.
 */
TEST_F(ConfigTest, WrongGroupAndKeyThrowsForString) {
    // Create a config source
    std::unique_ptr<ConfigSource> source(new StubbedConfigSource());

    // Put source in list so it can be passed to Config
    std::vector<std::unique_ptr<ConfigSource>> sources;
    sources.push_back(std::move(source));

    Config config(std::move(sources), MandatoryConfigs(), ConfigDependencies());

    ASSERT_THROW(config.getStringValue("WrongGroup", "wrong-key"), ConfigNotFoundError);
}

/*
 * Test that calling getters for non existing configs restults in the expected exception
 *
 * This test is for integers.
 */
TEST_F(ConfigTest, WrongGroupAndKeyThrowsForInt) {
    // Create a config source
    std::unique_ptr<ConfigSource> source(new StubbedConfigSource());

    // Put source in list so it can be passed to Config
    std::vector<std::unique_ptr<ConfigSource>> sources;
    sources.push_back(std::move(source));

    Config config(std::move(sources), MandatoryConfigs(), ConfigDependencies());

    ASSERT_THROW(config.getIntValue("WrongGroup", "wrong-key"), ConfigNotFoundError);
}

/*
 * Test that calling getters for non existing configs restults in the expected exception
 *
 * This test is for booleans.
 */
TEST_F(ConfigTest, WrongGroupAndKeyThrowsForBool) {
    // Create a config source
    std::unique_ptr<ConfigSource> source(new StubbedConfigSource());

    // Put source in list so it can be passed to Config
    std::vector<std::unique_ptr<ConfigSource>> sources;
    sources.push_back(std::move(source));

    Config config(std::move(sources), MandatoryConfigs(), ConfigDependencies());

    ASSERT_THROW(config.getBoolValue("WrongGroup", "wrong-key"), ConfigNotFoundError);
}

/*
 * Test that calling getters with wrong group but correct key throws an exception.
 * This should be treated in the same way as when using both wrong group and key.
 *
 * This test is only done for one type, it's unlikely that the behavior is different
 * for different types given that the above tests for "both group and key wrong" is
 * performed for all types.
 */
TEST_F(ConfigTest, WrongGroupThrows) {
    // Create a config source and add a config item to it
    std::unique_ptr<ConfigSource> source(new StubbedConfigSource());
    cast(source)->addConfig("MyGroup1", "my-key1", 1, ConfigSourceType::Main);

    // Put source in list so it can be passed to Config
    std::vector<std::unique_ptr<ConfigSource>> sources;
    sources.push_back(std::move(source));

    Config config(std::move(sources), MandatoryConfigs(), ConfigDependencies());

    ASSERT_THROW(config.getIntValue("WrongGroup", "my-key1"), ConfigNotFoundError);
}

/*
 * Test that calling getters with wrong key but correct group throws an exception.
 * This should be treated in the same way as when using both wrong group and key.
 *
 * This test is only done for one type, it's unlikely that the behavior is different
 * for different types given that the above tests for "both group and key wrong" is
 * performed for all types.
 */
TEST_F(ConfigTest, WrongKeyThrows) {
    // Create a config source and add a config item to it
    std::unique_ptr<ConfigSource> source(new StubbedConfigSource());
    cast(source)->addConfig("MyGroup1", "my-key1", 1, ConfigSourceType::Main);

    // Put source in list so it can be passed to Config
    std::vector<std::unique_ptr<ConfigSource>> sources;
    sources.push_back(std::move(source));

    Config config(std::move(sources), MandatoryConfigs(), ConfigDependencies());

    ASSERT_THROW(config.getIntValue("MyGroup1", "wrong-key1"), ConfigNotFoundError);
}

/*
 * Test that Config returns the value from the most prioritized source
 *
 * Two different sources are set to Config. Each source contain the same group-key combo but
 * with different values. The value from the most prioritized source is expected to be returned
 * by Config.
 *
 * ConfigSourceType::MainConfig has higher prio than ConfigSourceType::Defaults, i.e. the
 * value in the config provided by "StubbedMainConfig" should be used.
 */
TEST_F(ConfigTest, ExpectedSourcePrioTwoSources) {
    // Create a config source and add config items to it
    std::unique_ptr<ConfigSource> source(new StubbedConfigSource());
    cast(source)->addConfig("MyGroup1", "my-key1", std::string("myValue1"), ConfigSourceType::Main);
    cast(source)->addConfig("MyGroup1", "my-key1", std::string("myValue2"), ConfigSourceType::Default);
    cast(source)->addConfig("MyGroup1", "my-int-key1", 1, ConfigSourceType::Main);
    cast(source)->addConfig("MyGroup1", "my-int-key1", 2, ConfigSourceType::Default);
    cast(source)->addConfig("MyGroup1", "my-bool-key1", true, ConfigSourceType::Main);
    cast(source)->addConfig("MyGroup1", "my-bool-key1", false, ConfigSourceType::Default);

    // Put source in list so it can be passed to Config
    std::vector<std::unique_ptr<ConfigSource>> sources;
    sources.push_back(std::move(source));

    Config config(std::move(sources), MandatoryConfigs(), ConfigDependencies());

    // The values should be the ones from "Main" source since that takes precedence over
    // "Default" source
    ASSERT_EQ(config.getStringValue("MyGroup1", "my-key1"), "myValue1");
    ASSERT_EQ(config.getIntValue("MyGroup1", "my-int-key1"), 1);
    ASSERT_EQ(config.getBoolValue("MyGroup1", "my-bool-key1"), true);
}

/*
 * Same as above but using command line source as well
 */
TEST_F(ConfigTest, ExpectedSourcePrioThreeSources) {
    // Create a config source and add config items to it
    std::unique_ptr<ConfigSource> source(new StubbedConfigSource());
    cast(source)->addConfig("MyGroup1", "my-key1", std::string("myValue1"), ConfigSourceType::Main);
    cast(source)->addConfig("MyGroup1", "my-key1", std::string("myValue2"), ConfigSourceType::Default);
    cast(source)->addConfig("MyGroup1", "my-key1", std::string("myValue3"), ConfigSourceType::Commandline);
    cast(source)->addConfig("MyGroup1", "my-int-key1", 1, ConfigSourceType::Main);
    cast(source)->addConfig("MyGroup1", "my-int-key1", 2, ConfigSourceType::Default);
    cast(source)->addConfig("MyGroup1", "my-int-key1", 3, ConfigSourceType::Commandline);
    cast(source)->addConfig("MyGroup1", "my-bool-key1", true, ConfigSourceType::Main);
    cast(source)->addConfig("MyGroup1", "my-bool-key1", false, ConfigSourceType::Default);
    cast(source)->addConfig("MyGroup1", "my-bool-key1", false, ConfigSourceType::Commandline);

    // Put source in list so it can be passed to Config
    std::vector<std::unique_ptr<ConfigSource>> sources;
    sources.push_back(std::move(source));

    Config config(std::move(sources), MandatoryConfigs(), ConfigDependencies());

    // The values should be the ones from "Commandline" source since that takes precedence over
    // "Main", and "Default" sources
    ASSERT_EQ(config.getStringValue("MyGroup1", "my-key1"), "myValue3");
    ASSERT_EQ(config.getIntValue("MyGroup1", "my-int-key1"), 3);
    ASSERT_EQ(config.getBoolValue("MyGroup1", "my-bool-key1"), false);
}

/*
 * Test that a missing mandatory config throws an exception
 */
TEST_F(ConfigTest, MissingMandatoryConfigThrows) {
    // Create a config source and add a config item to it
    std::unique_ptr<ConfigSource> source(new StubbedConfigSource());
    cast(source)->addConfig("MyGroup1", "my-int-key1", 1, ConfigSourceType::Main);

    // Put source in list so it can be passed to Config
    std::vector<std::unique_ptr<ConfigSource>> sources;
    sources.push_back(std::move(source));

    MandatoryConfigs mandatory {UniqueKey("MissingGroup", "missing-key")};

    ASSERT_THROW(Config config(std::move(sources), mandatory, ConfigDependencies()), ConfigMandatoryError);
}

/*
 * Test that a present mandatory config does not throw an exception
 */
TEST_F(ConfigTest, PresentMandatoryConfigDontThrow) {
    // Create a config source and add a config item to it
    std::unique_ptr<ConfigSource> source(new StubbedConfigSource());
    cast(source)->addConfig("MyGroup1", "my-int-key1", 1, ConfigSourceType::Main);

    // Put source in list so it can be passed to Config
    std::vector<std::unique_ptr<ConfigSource>> sources;
    sources.push_back(std::move(source));

    MandatoryConfigs mandatory {UniqueKey("MyGroup1", "my-int-key1")};

    ASSERT_NO_THROW(Config config(std::move(sources), mandatory, ConfigDependencies()));
}

/*
 * Test that a missing dependency throws an exception
 */
TEST_F(ConfigTest, MissingDependencyThrows) {
    // Create a config source and add a config item to it
    std::unique_ptr<ConfigSource> source(new StubbedConfigSource());
    cast(source)->addConfig("MyGroup1", "my-int-key1", 1, ConfigSourceType::Main);

    // Put source in list so it can be passed to Config
    std::vector<std::unique_ptr<ConfigSource>> sources;
    sources.push_back(std::move(source));

    // Create a dependency and associate it with a present config. Since the
    // dependency config will not be present, an exception will be thrown
    ConfigDependencies deps {{UniqueKey("MyGroup1", "my-int-key1"), {UniqueKey("MissingGroup", "missing-key")}}};

    ASSERT_THROW(Config config(std::move(sources), MandatoryConfigs(), deps), ConfigDependencyError);
}

/*
 * Test that a present dependency does not throw an exception
 */
TEST_F(ConfigTest, PresentDependencyDontThrow) {
    // Create a config source and add a config item to it
    std::unique_ptr<ConfigSource> source(new StubbedConfigSource());
    cast(source)->addConfig("MyGroup1", "my-int-key1", 1, ConfigSourceType::Main);
    cast(source)->addConfig("MyGroup1", "my-int-key2", 2, ConfigSourceType::Default);

    // Put source in list so it can be passed to Config
    std::vector<std::unique_ptr<ConfigSource>> sources;
    sources.push_back(std::move(source));

    // Create a dependency and associate it with a present config. Since the
    // dependency config will be present, no exception needs to be thrown
    ConfigDependencies deps {{UniqueKey("MyGroup1", "my-int-key1"), {UniqueKey("MyGroup1", "my-int-key2")}}};

    ASSERT_NO_THROW(Config config(std::move(sources), MandatoryConfigs(), deps));
}



// MainConfigSourceTest //////////////////////////////////////////////////////////////////////////

class MainConfigSourceTest : public ::testing::Test
{
};


/*
 * Test that the expected config items are returned
 */
TEST_F(MainConfigSourceTest, ReturnsExpecedStringConfigs) {
    // Well formed config
    const std::string configString = "[SoftwareContainer]\n"
                                     "stringitem = mystring\n"
                                     "intitem = 1\n"
                                     "boolitem = true\n";

    std::unique_ptr<ConfigLoader> loader(new StringConfigLoader(configString));

    TypeMap typeMapping {{UniqueKey("SoftwareContainer", "stringitem"), ConfigType::String},
                         {UniqueKey("SoftwareContainer", "intitem"), ConfigType::Integer},
                         {UniqueKey("SoftwareContainer", "boolitem"), ConfigType::Boolean}};

    MainConfigSource source(std::move(loader), typeMapping);

    StringConfig expectedStringItem("SoftwareContainer", "stringitem", "mystring");
    IntConfig expectedIntItem("SoftwareContainer", "intitem", 1);
    BoolConfig expectedBoolItem("SoftwareContainer", "boolitem", true);

    std::vector<StringConfig> stringConfigs = source.stringConfigs();
    std::vector<IntConfig> intConfigs = source.intConfigs();
    std::vector<BoolConfig> boolConfigs = source.boolConfigs();

    ASSERT_EQ(stringConfigs.back().key(), expectedStringItem.key());
    ASSERT_EQ(stringConfigs.back().group(), expectedStringItem.group());
    ASSERT_EQ(stringConfigs.back().value(), expectedStringItem.value());

    ASSERT_EQ(intConfigs.back().key(), expectedIntItem.key());
    ASSERT_EQ(intConfigs.back().group(), expectedIntItem.group());
    ASSERT_EQ(intConfigs.back().value(), expectedIntItem.value());

    ASSERT_EQ(boolConfigs.back().key(), expectedBoolItem.key());
    ASSERT_EQ(boolConfigs.back().group(), expectedBoolItem.group());
    ASSERT_EQ(boolConfigs.back().value(), expectedBoolItem.value());
}

/*
 * Test that a config file with a config item that is not part of the type mapping
 * results in the expected exception
 */
TEST_F(MainConfigSourceTest, UnkownItemThrows) {
    // Well formed config that contains an item that will not be part of the type mapping
    const std::string configString = "[SoftwareContainer]\n"
                                     "sc.unknown=false\n";

    std::unique_ptr<ConfigLoader> loader(new StringConfigLoader(configString));

    // The type map now contains something that is not present in the config
    TypeMap typeMapping {{UniqueKey("SoftwareContainer", "sc.foo"), ConfigType::Boolean}};

    ASSERT_THROW(MainConfigSource(std::move(loader), typeMapping), ConfigUnknownError);
}

/*
 * Test that a config file with badly formatted data throws an exception
 */
TEST_F(MainConfigSourceTest, BadlyFormattedConfigThrows) {
    // Badly formatted config string (missing bracket in group)
    const std::string configString = "[SoftwareContainer\n"
                                     "sc.foo=false\n";

    std::unique_ptr<ConfigLoader> loader(new StringConfigLoader(configString));

    ASSERT_THROW(MainConfigSource(std::move(loader), TypeMap()), ConfigFileError);
}

/*
 * Test that a config with an unexpected value type retults in the expected exception
 */
TEST_F(MainConfigSourceTest, UnexpectedValueTypeThrows) {
    // Well formatted config string where the value is of an unexpected type, i.e.
    // the "foo" key needs to be parsed as a string but will be added to the type mapping
    // as an Integer type and thus, the parsing code path should result in an exception.
    const std::string configString = "[SoftwareContainer]\n"
                                     "foo = monkey\n";

    std::unique_ptr<ConfigLoader> loader(new StringConfigLoader(configString));

    TypeMap typeMapping {{UniqueKey("SoftwareContainer", "foo"), ConfigType::Integer}};

    ASSERT_THROW(MainConfigSource(std::move(loader), typeMapping), ConfigFileError);
}



// FileConfigLoaderTest //////////////////////////////////////////////////////////////////////////

class FileConfigLoaderTest : public ::testing::Test
{
};

/*
 * Test that using the real FileConfigLoader and passing a path to a non-existent
 * config file results in the expected exception FileError.
 *
 * The other possible exception from FileConfigLoader is KeyFileError when the file
 * exists but is not well formed, but that is not tested in the unit-tests becuase of the
 * dependency to having actual files.
 */
TEST_F(FileConfigLoaderTest, LoadingMissingConfigFileThrows) {
    FileConfigLoader loader("non-existent-path");

    ASSERT_THROW(loader.loadConfig(), Glib::FileError);
}
