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

#include "dbusgatewayparser.h"
#include "gatewayparsererror.h"

ReturnCode DBusGatewayParser::parseDBusConfig(const json_t *element,
                                              const char *key,
                                              json_t *config)
{
    log_debug() << "Parsing element for " << key;
    json_t *configExists = json_object_get(element, key);
    if (nullptr == configExists) {
        // This is not a fatal error - not providing the key for one of the buses is OK.
        log_error() << key << " was not found in config.";
        return ReturnCode::FAILURE;
    }

    if (!json_is_array(configExists)) {
        throwWithLog(logging::StringBuilder() << "Value for " << key << " is not an array");
    }

    for (unsigned int i = 0; i < json_array_size(configExists); i++) {
        json_t *child = json_array_get(configExists, i);
        if (!json_is_object(child)) {
            throwWithLog("JSON array element is not an object!");
        } else {
            json_array_append(config, json_deep_copy(child));
        }
    }

    return ReturnCode::SUCCESS;
}

void DBusGatewayParser::throwWithLog(std::string message)
{
    log_error() << message;
    throw GatewayParserError(message);
}
