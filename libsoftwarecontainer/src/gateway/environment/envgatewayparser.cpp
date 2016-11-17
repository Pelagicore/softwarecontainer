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

#include "envgatewayparser.h"

ReturnCode EnvironmentGatewayParser::parseEnvironmentGatewayConfigElement(
    const json_t *element,
    EnvironmentVariable &result,
    const EnvironmentVariables &store)
{

    if (isError(requireNonEmptyKeyValue(element, "name", result.first))
     || isError(requireNonEmptyKeyValue(element, "value", result.second))) {
        return ReturnCode::FAILURE;
    }

    bool appendMode = false;
    JSONParser::read(element, "append", appendMode); // This key is optional

    if (store.count(result.first) == 0) {
        if (appendMode) {
            log_info() << "Env variable \"" << result.first
                       << "\" was configured to be appended but the variable has not previously"
                       << " been set, so it will be created. Value is set to: \""
                       << result.second << "\"";
        }
        return ReturnCode::SUCCESS;
    } else {
        if (appendMode) {
            result.second = store.at(result.first) + result.second;
            return ReturnCode::SUCCESS;
        } else {
            log_error() << "Env variable " << result.first
                        << " already defined with value : " << store.at(result.first);
            return ReturnCode::FAILURE;
        }
    }
}

ReturnCode EnvironmentGatewayParser::requireNonEmptyKeyValue(const json_t *element,
                                                             const std::string key,
                                                             std::string &result)
{
    if (!JSONParser::read(element, key.c_str(), result)) {
        log_error() << "Key " << key << " missing or not a string in json configuration";
        return ReturnCode::FAILURE;
    }

    if (result.length() == 0) {
        log_error() << "Value for " << key << " key is an empty string";
        return ReturnCode::FAILURE;
    }

    return ReturnCode::SUCCESS;
}
