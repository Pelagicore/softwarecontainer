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

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "jansson.h"

template <typename T>
class GatewayParserCommon : public ::testing::TestWithParam<T>
{
public:
    
    json_error_t err;

    /**
     * @brief Converts a std::string to json_t* which is later cleaned up at teardown.
     */
    json_t *convertToJSON(const std::string &config)
    {
        json_t *jsonPtr = json_loads(config.c_str(), 0, &err);

        teardownAtEndOfTest(jsonPtr);
 
        return jsonPtr;
    }

    /**
     * @brief Makes sure that the given json_t is propperly cleaned up at the end of a test
     */
    void teardownAtEndOfTest(json_t *jsonPtr)
    {
        if (jsonPtr != nullptr) {
            jsonPtrs.push_back(jsonPtr);
        }
    }

    /**
     * @brief Cleanup the json
     */
    void TearDown() override
    {
        for(json_t *jsonPtr : jsonPtrs) {
            if (jsonPtr != nullptr) { 
                json_decref(jsonPtr);
            }
        }
        jsonPtrs.clear();
    }


private:
    std::vector<json_t *> jsonPtrs;

};
