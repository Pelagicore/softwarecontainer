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

#include "jsonparser.h"

namespace softwarecontainer {

bool JSONParser::read(const json_t *element, const char *key, std::string &result)
{
    json_t *value = json_object_get(element, key);
    if (!value) {
        log_error() << "Could not fetch \"" << key << "\" from json element";
        return false;
    }

    if (!json_is_string(value)) {
        log_error() << "json element is not a string";
        return false;
    }

    result = json_string_value(value);
    return true;
}

bool JSONParser::read(const json_t *element, const char *key, bool &result)
{
    json_t *value = json_object_get(element, key);
    if (!value) {
        log_error() << "Could not fetch \"" << key << "\" from json element";
        return false;
    }

    if (!json_is_boolean(value)) {
        log_error() << "json element is not a boolean";
        return false;
    }

    result = json_is_true(value);
    return true;
}

bool JSONParser::read(const json_t *element, const char *key, int &result)
{
    json_t *value = json_object_get(element, key);
    if (!value) {
        log_error() << "Could not fetch \"" << key << "\" from json element";
        return false;
    }

    if (!json_is_integer(value)) {
        log_error() << "json element is not an integer";
        return false;
    }

    result = json_integer_value(value);
    return true;
}

}
