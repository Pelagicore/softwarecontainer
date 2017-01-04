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

#include "cgroupsparser.h"
#include "jsonparser.h"

namespace softwarecontainer {

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

} // namespace softwarecontainer
