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
#pragma once

#include "softwarecontainer-log.h"
#include <jansson.h>

namespace softwarecontainer {

    /**
     * @brief Convenience layer for the Jansson JSON parser.
     *
     * This class provides convenience functions that handles commonly used operations
     * using the Janssson JSON parser.
     */
    class JSONParser final {
        LOG_DECLARE_CLASS_CONTEXT("JSON", "JSONParser");

    public:
        /**
         * @brief Reads a string from a JSON Object.
         *
         * Tries to read a string value form a given JSON object using a given key.
         * If the read is successful true will be returned and the result reference will be
         * populated with the read value. If the string could not be read, either due to the
         * given object does not contain the given key or the given key did not hold a string,
         * false will be returned and the result reference will not be changed.
         *
         * @return True: If the string was successfully read.
         * @return False: If the string could not be read.
         */
        static bool read(const json_t *element, const char *key, std::string &result);

        /**
         * @brief Reads a boolean from a JSON Object.
         *
         * Tries to read a boolean value form a given JSON object using a given key.
         * If the read is successful true will be returned and the result reference will be
         * populated with the read value. If the boolean could not be read, either due to the
         * given object does not contain the given key or the given key did not hold a boolean
         * value, false will be returned and the result reference will not be changed.
         *
         * @return True: If the boolean was successfully read.
         * @return False: If the boolean could not be read.
         */
        static bool read(const json_t *element, const char *key, bool &result);

        /**
         * @brief Reads an integer from a JSON Object.
         *
         * Tries to read an integer value form a given JSON object using a given key.
         * If the read is successful true will be returned and the result reference will be
         * populated with the read value. If the integer could not be read, either due to the
         * given object does not contain the given key or the given key did not hold an integer
         * value, false will be returned and the result reference will not be changed.
         *
         * @return True: If the integer was successfully read.
         * @return False: If the integer could not be read.
         */
        static bool read(const json_t *element, const char *key, int &result);

        /**
         * @brief Checks if a given JSON object contains a certain key
         */
        static bool hasKey(const json_t *element, const char *key);
    };
}
