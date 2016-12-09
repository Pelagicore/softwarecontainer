
#pragma once

#include "softwarecontainer-common.h"
#include <jansson.h>

class CGroupsParser
{
    LOG_DECLARE_CLASS_CONTEXT("CGPA", "CGroups Gateway Parser");

public:
    typedef std::pair< std::string, std::string > CGroupsPair;

    ReturnCode parseCGroupsGatewayConfiguration(const json_t *element, CGroupsPair &result);

};
