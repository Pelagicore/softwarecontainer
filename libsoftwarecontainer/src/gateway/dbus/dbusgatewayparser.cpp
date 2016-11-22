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


ReturnCode DBusGatewayParser::parseDBusConfigElement(const json_t *element,
                                                     json_t *session,
                                                     json_t *system)
{

    if (!JSONParser::hasKey(element, SESSION_CONFIG)
     && !JSONParser::hasKey(element, SYSTEM_CONFIG)) {
        log_error() << "Neither system nor session configuration was provided";
        return ReturnCode::FAILURE;
    }

    if (isError(parseDBusBusConfig(element, SESSION_CONFIG, session))) {
        return ReturnCode::FAILURE;
    }

    if (isError(parseDBusBusConfig(element, SYSTEM_CONFIG, system))) {
        return ReturnCode::FAILURE;
    }

    return ReturnCode::SUCCESS;
}

ReturnCode DBusGatewayParser::parseDBusBusConfig(const json_t *element,
                                                 const char *key,
                                                 json_t *config)
{
    json_t *configExists = json_object_get(element, key);
    if (configExists) {
        if (!json_is_array(configExists)) {
            log_error() << "Value for " << key << " is not an array";
            return ReturnCode::FAILURE;
        }

        for (unsigned int i = 0; i < json_array_size(configExists); i++) {
            json_t *child = json_array_get(configExists, i);
            // TODO: Error checking here, so that dbus-proxy can accept config
            json_array_append(config, json_deep_copy(child));
        }
    }

    return ReturnCode::SUCCESS;
}
