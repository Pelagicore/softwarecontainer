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

#include <glibmm.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <unistd.h>
#include <libgen.h>

using namespace softwarecontainer;

class ConfigStoreTest: public ::testing::Test
{
public:
    LOG_DECLARE_CLASS_CONTEXT("TEST", "Tester");
    ConfigStoreTest() {}

    std::string capNameA = "com.pelagicore.temperatureservice.gettemperature";
    std::string capNameB = "com.pelagicore.temperatureservice.settemperature";
    std::string capNameC = "dummyCapC";
    std::string capNameD = "dummyCapD";

    /* The Service Manifests' (relative) file paths */
    const std::string testDataDir        = std::string(TEST_DATA_DIR);
    const std::string dirPath            = buildPath(testDataDir, "dirNoManifests");
    const std::string onlyValidDir       = buildPath(testDataDir, "onlyValidManifests");
    const std::string fileErrorDir       = buildPath(testDataDir, "fileErrorManifests");
    const std::string parseErrorDir      = buildPath(testDataDir, "parseErrorManifests");
    const std::string manifestPath       = "CS_unittest_ServiceManifest.json";
    const std::string shortManifestPath  = "CS_unittest_short_ServiceManifest.json";
    const std::string evilManifest       = "CS_unittest_parseError.json";
    const std::string notJSONManifest    = "CS_unittest_fileExtensionIsNotJson.txt";
    const std::string notJSONManifest2   = "CS_unittest_fileIsNotJson.json";
    const std::string nonExistingPath    = "/home/aiuaiai/iuatxia";

    // TODO: (thenor) Do not use files, write string to file
    const std::string parseNoJsonObj        = "CS_unittest_parseErrorNoJsonObject.json";
    const std::string parseOneCapNOK        = "CS_unittest_parseErrorOneCapNotOk.json";
    const std::string parseCapNameNOK       = "CS_unittest_parseErrorCapNameNOK.json";
    const std::string parseGWObjNOK         = "CS_unittest_parseErrorGWObjNOK.json";
    const std::string parseGWNotArray       = "CS_unittest_parseErrorGWNotArray.json";
    const std::string parseGWNotJson        = "CS_unittest_parseErrorGWNotJsonObject.json";
    const std::string parseGWIdNOK          = "CS_unittest_parseErrorGWIdNOK.json";
    const std::string parseGWConfigNOK      = "CS_unittest_parseErrorGWConfigNOK.json";
    const std::string parseGWConfigNotArray = "CS_unittest_parseErrorGWConfigNotArray.json";
};

/* Constructing a BaseConfigStore with an empty file path should not throw an exception.
 */
TEST_F(ConfigStoreTest, constructorEmptyStrOk) {
    ASSERT_NO_THROW(BaseConfigStore(""));
}

/* Constructing a BaseConfigStore with a file path,
 * pointing at a parsable json file, should not throw an exception.
 */
TEST_F(ConfigStoreTest, constructorFileOk) {
    ASSERT_NO_THROW(BaseConfigStore(buildPath(testDataDir, shortManifestPath)));
}

/* Constructing a BaseConfigStore with a file path,
 * pointing at a parsable file, should not throw an exception.
 */
TEST_F(ConfigStoreTest, constructorFileOk2) {
    ASSERT_NO_THROW(BaseConfigStore(buildPath(testDataDir, manifestPath)));
}

/* Constructing a FilteredConfigStore with a directory path,
 * even if all files can not be parsed, should not throw an exception.
 */
TEST_F(ConfigStoreTest, constructorDirNoJsonFiles) {
    // No Service Manifests, but ok dir
    // TODO: Get rid of this + ""
    ASSERT_NO_THROW(FilteredConfigStore(dirPath + ""));
}

/* Constructing a BaseConfigStore with a directory path,
 * when the directory does not exist, should throw an exception of type ServiceManifestPathError.
 */
TEST_F(ConfigStoreTest, dirDoesNotExist) {
    // TODO: Get rid of this + ""
    ASSERT_THROW(BaseConfigStore(nonExistingPath + ""), ServiceManifestPathError);
}

/* Constructing a FilteredConfigStore with a directory path,
 * when the directory is "/", should throw an exception of type ServiceManifestPathError.
 */
TEST_F(ConfigStoreTest, rootDirNotAllowed) {
    ASSERT_THROW(FilteredConfigStore("/"), ServiceManifestPathError);
}

/* Constructing a BaseConfigStore with a file path
 * pointing at a Service Manifest which doesn't have the file extension "json"
 * should throw an exception of type ServiceManifestParseError.
 */
TEST_F(ConfigStoreTest, fileExtensionIsNotJson) {
    ASSERT_THROW(BaseConfigStore(buildPath(fileErrorDir, notJSONManifest)),
                 ServiceManifestParseError);
}

/* Constructing a BaseConfigStore with a file path
 * pointing at a Service Manifest which can not be parsed
 * due to that the file is not a json file
 * should throw an exception of type ServiceManifestParseError.
 */
TEST_F(ConfigStoreTest, fileIsNotJsonFile) {
    ASSERT_THROW(BaseConfigStore(buildPath(fileErrorDir, notJSONManifest2)),
                 ServiceManifestParseError);
}

/* Constructing a BaseConfigStore with a file path,
 * pointing at a Service Manifest which can not be parsed
 * due to that the Capability object is not a json array,
 * should throw an exception of type CapabilityParseError.
 */
TEST_F(ConfigStoreTest, capabilitiesIsNotAJsonArray) {
    ASSERT_THROW(BaseConfigStore(buildPath(testDataDir, evilManifest)), CapabilityParseError);
}

/* Constructing a BaseConfigStore with a directory path,
 * that contains a file that can't be parsed results in an exception.
 */
TEST_F(ConfigStoreTest, dirOneFileNotOk) {
    // TODO: Get rid of this + ""
    ASSERT_THROW(BaseConfigStore(testDataDir + ""), CapabilityParseError);
}

/* Reading gateway configurations from a Service Manifest file
 * and attempting to read an existing capabilities configs
 * should result in a non-empty result.
 */
TEST_F(ConfigStoreTest, readConfigFetchOneCap) {
    FilteredConfigStore cs = FilteredConfigStore(buildPath(testDataDir, manifestPath));

    GatewayConfiguration retGWs = cs.configByID(capNameA);
    ASSERT_FALSE(retGWs.empty());
}

/* Getting all capability names after reading a service manifest should
 * result in getting all those names that are listed in the manifest file.
 */
TEST_F(ConfigStoreTest, readConfigFetchAllCaps) {
    FilteredConfigStore cs = FilteredConfigStore(buildPath(testDataDir, manifestPath));

    std::vector<std::string> ids = cs.IDs();
    ASSERT_FALSE(ids.empty());
    ASSERT_NE(std::find(ids.begin(), ids.end(), capNameA), ids.end());
    ASSERT_NE(std::find(ids.begin(), ids.end(), capNameB), ids.end());
    ASSERT_NE(std::find(ids.begin(), ids.end(), capNameC), ids.end());
    ASSERT_NE(std::find(ids.begin(), ids.end(), capNameD), ids.end());
}

/* Reading gateway configurations from a Service Manifest file
 * and attempting to read an existing capabilities configs
 * should result in an empty result.
 */
TEST_F(ConfigStoreTest, readConfigFetchEvilCap) {
    FilteredConfigStore cs = FilteredConfigStore(buildPath(testDataDir, manifestPath));
    GatewayConfiguration retGWs = cs.configByID("EvilCapName");
    ASSERT_TRUE(retGWs.empty());
}

/* Reading gateway configurations from a Service Manifest file
 * and fetching a gateway configuration for an existing capability
 * should result in a non-empty result. Match result.
 */
TEST_F(ConfigStoreTest, readConfigFetchCapMatchConfig) {
    FilteredConfigStore cs = FilteredConfigStore(buildPath(testDataDir, shortManifestPath));
    GatewayConfiguration retGWs = cs.configByID(capNameA);
    EXPECT_FALSE(retGWs.empty());

    std::string configStr =
        "[{"
                "\"dbus-gateway-config-session\": []"
            "}, {"
                "\"dbus-gateway-config-system\": [{"
                    "\"direction\": \"outgoing\","
                    "\"interface\": \"org.freedesktop.DBus.Introspectable\","
                    "\"object-path\": \"/com/pelagicore/TemperatureService\","
                    "\"method\": \"Introspect\""
                "}]"
            "}]";
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

/* Reading gateway configurations from several Service Manifest files,
 * which contain gateways with the same IDs, should result in a merged result
 * when fetching the gateway configuration for an existing capability.
 * Match result.
 * Since this test reads an entire directory of files it is expected that one
 * reading of file results in a warning (for the "parseError" file).
 */
TEST_F(ConfigStoreTest, readConfigFetchCapMatchCombinedConfig) {
    FilteredConfigStore cs = FilteredConfigStore(onlyValidDir);
    GatewayConfiguration retGWs = cs.configByID(capNameA);
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

/*
TEST_F(ConfigStoreTest, parseErrorNotJson) {
    // Create a service manifest matching this error case
    //  - "The Service Manifest does not contain a json object "
    ASSERT_THROW(BaseConfigStore(buildPath(parseErrorDir, parseNoJsonObj)),
                 ServiceManifestParseError);
}
*/

TEST_F(ConfigStoreTest, parseErrorCapsObject) {
    // Create a service manifest matching this error case
    //  - "Could not parse the \"capability\" object in file: "
    ASSERT_THROW(BaseConfigStore(buildPath(parseErrorDir, parseNoJsonObj)),
                 CapabilityParseError);
}


// Parse caps
/*
TEST_F(ConfigStoreTest, parseErrorOneCapIsNotJson) {
    // Create a service manifest matching this error case
    //  - "A "capability" in the Service Manifest is not a json object"
    ASSERT_THROW(BaseConfigStore(buildPath(parseErrorDir, parseOneCapNOK)),
                 CapabilityParseError);

}
*/

TEST_F(ConfigStoreTest, parseErrorCapNameUnreadable) {
    // Create a service manifest matching this error case
    //  - "Could not read the name of the "capability" object"
    ASSERT_THROW(BaseConfigStore(buildPath(parseErrorDir, parseCapNameNOK)),
                 CapabilityParseError);
}

TEST_F(ConfigStoreTest, parseErrorGWElementUnreadable) {
    // Create a service manifest matching this error case
    //  - "Could not read the "gateway" objects in [capName]
    ASSERT_THROW(BaseConfigStore(buildPath(parseErrorDir, parseGWObjNOK)),
                 CapabilityParseError);
}

TEST_F(ConfigStoreTest, parseErrorGWElementIsNotJson) {
    // Create a service manifest matching this error case
    //  - "The "gateway" object is not an array"
    ASSERT_THROW(BaseConfigStore(buildPath(parseErrorDir, parseGWNotArray)),
                 CapabilityParseError);
}


/*
TEST_F(ConfigStoreTest, parseErrorGWElementKeyIsNotJson) {
    // Create a service manifest matching this error case
    //  - "The \"gateway\" key in the Service Manifest is not a json object"
    ASSERT_THROW(BaseConfigStore(buildPath(parseErrorDir, parseGWNotJson)),
                 CapabilityParseError);
}
*/
TEST_F(ConfigStoreTest, parseErrorGWElementIdUnreadable) {
    // Create a service manifest matching this error case
    //  - "Could not read the ID of the \"gateway\" object in the Service Manifest"
    ASSERT_THROW(BaseConfigStore(buildPath(parseErrorDir, parseGWIdNOK)),
                 CapabilityParseError);
}

TEST_F(ConfigStoreTest, parseErrorGWElementConfigUnreadable) {
    // Create a service manifest matching this error case
    //  - "Could not read the \"gateway\" object's configuration element (gwID)"
    ASSERT_THROW(BaseConfigStore(buildPath(parseErrorDir, parseGWConfigNOK)),
                 CapabilityParseError);
}

TEST_F(ConfigStoreTest, parseErrorGWElementConfigIsNotJsonArray) {
    // Create a service manifest matching this error case
    //  - "The \"gateway\" object's configuration is not an array (gwID)"
    ASSERT_THROW(BaseConfigStore(buildPath(parseErrorDir, parseGWConfigNotArray)),
                 CapabilityParseError);
}
