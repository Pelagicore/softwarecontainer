
#include "cgroupsparser.h"
#include "jsonparser.h"

ReturnCode CGroupsParser::parseCGroupsGatewayConfiguration(const json_t *element, CGroupsPair &result)
{
    std::string settingKey;
    std::string settingValue;

    if (!JSONParser::read(element, "setting", settingKey)) {
        log_error() << "Key \"setting\" either not a string or not in json configuration";
        return ReturnCode::FAILURE;
    }

    if (!JSONParser::read(element, "value", settingValue)) {
        log_error() << "Key \"value\" either not a string or not in json configuration";
        return ReturnCode::FAILURE;
    }

    result.first = settingKey;
    result.second = settingValue;
    return ReturnCode::SUCCESS;
}
