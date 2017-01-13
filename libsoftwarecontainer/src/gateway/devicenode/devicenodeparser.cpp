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

#include "devicenodeparser.h"

namespace softwarecontainer {

ReturnCode DeviceNodeParser::parseDeviceNodeGatewayConfiguration(const json_t *element,
                                                                 Device &result)
{
    result.name = "";
    if (!JSONParser::read(element, "name", result.name)) {
        log_error() << "Key \"name\" missing or not a string in json configuration";
        return ReturnCode::FAILURE;
    }

    result.major = -1;
    result.minor = -1;
    result.mode = -1;
    const bool majorSpecified = JSONParser::read(element, "major", result.major);
    const bool minorSpecified = JSONParser::read(element, "minor", result.minor);
    const bool modeSpecified = JSONParser::read(element, "mode",  result.mode);

    if (majorSpecified | minorSpecified) {
        return bool2ReturnCode(
               checkBoolSet(majorSpecified, "Major has to be specified when Minor is specified")
            && checkBoolSet(minorSpecified, "Minor has to be specified when Major is specified")
            && checkBoolSet(modeSpecified, "Mode has to be specified when Major and Minor are specified")
        );
    } 
    return ReturnCode::SUCCESS;
}

bool DeviceNodeParser::checkBoolSet(const bool &value, std::string errorMessage)
{
    if (!value) {
        log_error() << errorMessage;
        return false;
    }

    return true;
}

} // namespace softwarecontainer
