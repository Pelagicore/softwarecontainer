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

#include "capability/baseconfigstore.h"
#include "capability/filteredconfigstore.h"
#include "capability/defaultconfigstore.h"
#include "capability/servicemanifestloader.h"

#include <glibmm.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <unistd.h>
#include <libgen.h>

using namespace softwarecontainer;

/*
 * Test stub - ServiceManifestStringLoader
 *
 * Loads a service manifests from a string, as opposed to the non-test loader
 * which reads from file.
 */
class ServiceManifestStringLoader : public ServiceManifestLoader
{
LOG_DECLARE_CLASS_CONTEXT("SMSL", "SoftwareContainer service manifest loader");

public:
    // Constructor needs to init parent with the service manifest source string
    ServiceManifestStringLoader(const std::string &source) :
        ServiceManifestLoader(source)
    {}

    std::vector<json_t *> loadContent() override
    {
        if (m_source.empty()) {
            log_debug() << "Service manifests (m_source) is empty";
            return m_content;
        }
        return addContent(m_source);
    }

    std::vector<json_t *> addContent(const std::string &sourceString)
    {
        std::string errorMessage;
        json_error_t error;
        json_t *content = json_loads(sourceString.c_str(), 0, &error);

        if (nullptr == content) {
            errorMessage = "Could not parse the Service Manifest string: "
                + std::to_string(error.line) + " : " + std::string(error.text);
            log_error() << errorMessage;
            throw ServiceManifestParseError(errorMessage);
        } else if (!json_is_object(content)) {
            errorMessage = "The Service Manifest string does not contain a json object: "
                + std::to_string(error.line) + " : " + std::string(error.text);
            log_error() << errorMessage;
            throw ServiceManifestParseError(errorMessage);
        }

        m_content.push_back(content);

        return m_content;
    }
};


class ConfigStoreTest: public ::testing::Test
{
public:
    LOG_DECLARE_CLASS_CONTEXT("TEST", "Tester");
    ConfigStoreTest() {}

    const std::string capNameGetTemp = "com.pelagicore.temperatureservice.gettemperature";
    const std::string capNameSetTemp = "com.pelagicore.temperatureservice.settemperature";
    const std::string capNameC = "dummyCapC";
    const std::string capNameD = "dummyCapD";

    /* The building blocks for making service manifests */
    const std::string serviceManifestStart = "\"version\": \"1\",\"capabilities\": ";

    const std::string capGetTempStart =
        "\"name\": \"" + capNameGetTemp + "\",\"gateways\": ";
    const std::string capSetTempStart =
        "\"name\": \"" + capNameSetTemp + "\",\"gateways\": ";

    const std::string gwDbusStart = "\"id\": \"dbus\",\"config\": ";
    const std::string gwCfgSessionEmpty = "{\"dbus-gateway-config-session\": []}";
    const std::string gwCfgDbusSystemStart = "\"dbus-gateway-config-system\": ";
    const std::string gwCfgDbusIntrospect =
        "{\"direction\": \"outgoing\","
         "\"interface\": \"org.freedesktop.DBus.Introspectable\","
         "\"object-path\": \"/com/pelagicore/TemperatureService\","
         "\"method\": \"Introspect\"}";

    const std::string gwCfgDbusEcho =
        "{\"direction\": \"outgoing\","
         "\"interface\": \"com.pelagicore.TemperatureService\","
         "\"object-path\": \"/com/pelagicore/TemperatureService\","
         "\"method\": \"Echo\"}";
    const std::string gwCfgDbusGetTemp =
        "{\"direction\": \"outgoing\","
         "\"interface\": \"com.pelagicore.TemperatureService\","
         "\"object-path\": \"/com/pelagicore/TemperatureService\","
         "\"method\": \"GetTemperature\""
        "}, {\"direction\": \"incoming\","
         "\"interface\": \"com.pelagicore.TemperatureService\","
         "\"object-path\": \"/com/pelagicore/TemperatureService\","
         "\"method\": \"TemperatureChanged\"}";
    const std::string gwCfgDbusSetTemp =
        "{\"direction\": \"outgoing\","
          "\"interface\": \"com.pelagicore.TemperatureService\","
          "\"object-path\": \"/com/pelagicore/TemperatureService\","
          "\"method\": \"SetTemperature\"}";
    const std::string gwCfgDummy = "{\"id\": \"dummy-gw2\", \"config\": []}";

    /* Valid capability objects */
    const std::string capDummyC =
        "{\"name\": \"" + capNameC + "\","
         "\"gateways\": [{\"id\": \"dbus\",\"config\": []}, "
                        "{\"id\": \"dummy-gw2\",\"config\": []}]}";
    const std::string capDummyD =
        "{\"name\": \"" + capNameD + "\","
         "\"gateways\": [{\"id\": \"dummy-gw1\",\"config\": []}, "
                        "{\"id\": \"dummy-gw2\",\"config\": []}]}";
    const std::string capGetTempShort =
        "{" + capGetTempStart + "[{" + gwDbusStart + "[" + gwCfgSessionEmpty + ","
        + "{" + gwCfgDbusSystemStart + "[" + gwCfgDbusIntrospect +"]}"
        + "]}]}";
    const std::string capGetTempLong =
        "{" + capGetTempStart + "[{" + gwDbusStart + "[" + gwCfgSessionEmpty + ","
        + "{" + gwCfgDbusSystemStart  + "[" + gwCfgDbusIntrospect + ","
              + gwCfgDbusEcho + "," + gwCfgDbusGetTemp + "]}]}"
        + "," + gwCfgDummy + "]}";
    const std::string capSetTempLong =
        "{" + capSetTempStart + "[{" + gwDbusStart + "[" + gwCfgSessionEmpty + ","
        + "{" + gwCfgDbusSystemStart  + "[" + gwCfgDbusIntrospect
        + "," + gwCfgDbusSetTemp + "]}]}]}";

    /* Valid service manifests */
    const std::string shortManifest =
        "{" + serviceManifestStart + "[" + capGetTempShort + "]}";
    const std::string longManifest =
        "{" + serviceManifestStart + "[" + capGetTempLong + "," + capSetTempLong + ","
        + capDummyC + "," + capDummyD + "]}";

};

/* Constructing a BaseConfigStore with an empty file path should not throw an exception.
 */

TEST_F(ConfigStoreTest, constructorEmptyStrOk) {
    std::unique_ptr<ServiceManifestStringLoader> loader(new ServiceManifestStringLoader(""));
    ASSERT_NO_THROW(BaseConfigStore(std::move(loader)));
}

/* Constructing a BaseConfigStore with a file path,
 * pointing at a parsable json file, should not throw an exception.
 */
TEST_F(ConfigStoreTest, constructorFileOk) {
    // log_debug() << shortManifest;

    std::unique_ptr<ServiceManifestStringLoader> loader(new ServiceManifestStringLoader(shortManifest));
    ASSERT_NO_THROW(BaseConfigStore(std::move(loader)));
}

/* Constructing a BaseConfigStore with a file path,
 * pointing at a parsable file, should not throw an exception.
 */
TEST_F(ConfigStoreTest, constructorFileOk2) {
    // log_debug() << longManifest;
    std::unique_ptr<ServiceManifestStringLoader> loader(new ServiceManifestStringLoader(longManifest));
    ASSERT_NO_THROW(BaseConfigStore(std::move(loader)));
}

/* Constructing a BaseConfigStore with a file path,
 * pointing at a Service Manifest which can not be parsed
 * due to that the Capability object is not a json array,
 * should throw an exception of type CapabilityParseError.
 */
TEST_F(ConfigStoreTest, capabilitiesIsNotAJsonArray) {
    const std::string capIsNotArray =
        "{" + serviceManifestStart + "1234 }";

    std::unique_ptr<ServiceManifestStringLoader> loader(new ServiceManifestStringLoader(capIsNotArray));
    ASSERT_THROW(BaseConfigStore(std::move(loader)), CapabilityParseError);
}

/* Reading gateway configurations from a Service Manifest file
 * and attempting to read an existing capabilities configs
 * should result in a non-empty result.
 */
TEST_F(ConfigStoreTest, readConfigFetchOneCap) {
    std::unique_ptr<ServiceManifestStringLoader> loader(new ServiceManifestStringLoader(longManifest));
    FilteredConfigStore cs = FilteredConfigStore(std::move(loader));

    GatewayConfiguration retGWs = cs.configByID(capNameGetTemp);
    ASSERT_FALSE(retGWs.empty());
}

/* Getting all capability names after reading a service manifest should
 * result in getting all those names that are listed in the manifest file.
 */
TEST_F(ConfigStoreTest, readConfigFetchAllCaps) {
    std::unique_ptr<ServiceManifestStringLoader> loader(new ServiceManifestStringLoader(longManifest));
    FilteredConfigStore cs = FilteredConfigStore(std::move(loader));

    std::vector<std::string> ids = cs.IDs();
    ASSERT_FALSE(ids.empty());
    ASSERT_NE(std::find(ids.begin(), ids.end(), capNameGetTemp), ids.end());
    ASSERT_NE(std::find(ids.begin(), ids.end(), capNameSetTemp), ids.end());
    ASSERT_NE(std::find(ids.begin(), ids.end(), capNameC), ids.end());
    ASSERT_NE(std::find(ids.begin(), ids.end(), capNameD), ids.end());
}

/* Reading gateway configurations from a Service Manifest file
 * and attempting to read an existing capabilities configs
 * should result in an empty result.
 */
TEST_F(ConfigStoreTest, readConfigFetchEvilCap) {
    std::unique_ptr<ServiceManifestStringLoader> loader(new ServiceManifestStringLoader(longManifest));
    FilteredConfigStore cs = FilteredConfigStore(std::move(loader));
    GatewayConfiguration retGWs = cs.configByID("EvilCapName");
    ASSERT_TRUE(retGWs.empty());
}

/* Reading gateway configurations from a Service Manifest file
 * and fetching a gateway configuration for an existing capability
 * should result in a non-empty result. Match result.
 */
TEST_F(ConfigStoreTest, readConfigFetchCapMatchConfig) {
    std::unique_ptr<ServiceManifestStringLoader> loader(new ServiceManifestStringLoader(shortManifest));
    FilteredConfigStore cs = FilteredConfigStore(std::move(loader));
    GatewayConfiguration retGWs = cs.configByID(capNameGetTemp);
    EXPECT_FALSE(retGWs.empty());

    std::string configStr = "[" + gwCfgSessionEmpty + ", {"
        + gwCfgDbusSystemStart  + "[" + gwCfgDbusIntrospect + "]}]";
    json_error_t error;
    json_t *expectedJson = json_loads(configStr.c_str(), 0, &error);
    EXPECT_FALSE(nullptr == expectedJson);

    std::string gwID = "dbus";
    json_t *retGWConfigs = retGWs.config(gwID);
    ASSERT_TRUE(json_is_array(retGWConfigs));
    ASSERT_TRUE(json_equal(retGWConfigs,expectedJson));

    json_decref(expectedJson);
    json_decref(retGWConfigs);
}

/* Reading gateway configurations from several Service Manifests,
 * which contain gateways with the same IDs, should result in a merged result
 * when fetching the gateway configuration for an existing capability.
 * Match result.
 * Since this test reads an entire directory of files it is expected that one
 * reading of file results in a warning (for the "parseError" file).
 */
TEST_F(ConfigStoreTest, readConfigFetchCapMatchCombinedConfig) {
    std::unique_ptr<ServiceManifestStringLoader> loader(new ServiceManifestStringLoader(shortManifest));
    // This is used to simulate the behavior of ConfigStore when several service manifests
    // and combining the gateway objects
    loader->addContent(longManifest);

    FilteredConfigStore cs = FilteredConfigStore(std::move(loader));
    GatewayConfiguration retGWs = cs.configByID(capNameGetTemp);
    EXPECT_FALSE(retGWs.empty());
    std::vector<json_t *> expectedDbusConfigs;

    std::string configStr1 =
        "{\"dbus-gateway-config-session\": []}";
    std::string configStr2 =
        "{\"dbus-gateway-config-system\": [{"
               "\"direction\": \"outgoing\","
               "\"interface\": \"org.freedesktop.DBus.Introspectable\","
               "\"object-path\": \"/com/pelagicore/TemperatureService\","
               "\"method\": \"Introspect\""
            "}, {"
               "\"direction\": \"outgoing\","
               "\"interface\": \"com.pelagicore.TemperatureService\","
               "\"object-path\": \"/com/pelagicore/TemperatureService\","
               "\"method\": \"Echo\""
            "}, {"
               "\"direction\": \"outgoing\","
               "\"interface\": \"com.pelagicore.TemperatureService\","
               "\"object-path\": \"/com/pelagicore/TemperatureService\","
               "\"method\": \"GetTemperature\""
            "}, {"
               "\"direction\": \"incoming\","
               "\"interface\": \"com.pelagicore.TemperatureService\","
               "\"object-path\": \"/com/pelagicore/TemperatureService\","
               "\"method\": \"TemperatureChanged\""
        "}]}";
    std::string configStr3 =
        "{\"dbus-gateway-config-system\": [{"
               "\"direction\": \"outgoing\","
               "\"interface\": \"org.freedesktop.DBus.Introspectable\","
               "\"object-path\": \"/com/pelagicore/TemperatureService\","
               "\"method\": \"Introspect\""
        "}]}";
    json_error_t error;
    json_t *expectedJson1 = json_loads(configStr1.c_str(), 0, &error);
    EXPECT_FALSE(nullptr == expectedJson1);
    expectedDbusConfigs.push_back(expectedJson1);

    json_t *expectedJson2 = json_loads(configStr2.c_str(), 0, &error);
    EXPECT_FALSE(nullptr == expectedJson2);
    expectedDbusConfigs.push_back(expectedJson2);

    json_t *expectedJson3 = json_loads(configStr3.c_str(), 0, &error);
    EXPECT_FALSE(nullptr == expectedJson3);
    expectedDbusConfigs.push_back(expectedJson3);

    std::string gwID = "dbus";
    json_t *retGWConfigs = retGWs.config(gwID);
    ASSERT_TRUE(json_is_array(retGWConfigs));

    // Match that all expected GW configs are present in the returned GW configs
    size_t i;
    json_t *config;
    json_array_foreach(retGWConfigs, i, config) {
        bool retval = false;
        ASSERT_TRUE(json_is_object(config));

        for (json_t *expectedConfig : expectedDbusConfigs) {
            if (json_equal(config, expectedConfig)) {
                retval = true;
            }
        }
        // If one of the expected values exist in the result
        // the retval will have been set to 'true'
        ASSERT_TRUE(retval);
    }
    json_decref(expectedJson1);
    json_decref(expectedJson2);
    json_decref(expectedJson3);
    json_decref(retGWConfigs);
}
