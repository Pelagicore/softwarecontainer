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

bool DeviceNodeParser::parseDeviceNodeGatewayConfiguration(const json_t *element,
                                                                 Device &result)
{
    result.name = "";
    if (!JSONParser::read(element, "name", result.name)) {
        log_error() << "Key \"name\" missing or not a string in json configuration";
        return false;
    }

    result.mode = -1;

    // Mode is optional, i.e. do not do anything if it is not specified.
    if (nullptr != json_object_get(element, "mode")) {
        const bool modeParses = JSONParser::read(element, "mode", result.mode);

        if (!modeParses) {
            log_error() << "Mode specified with bad format";
            return false;
        }
    }

    result.isConfigured = false;
    return true;
}

} // namespace softwarecontainer
