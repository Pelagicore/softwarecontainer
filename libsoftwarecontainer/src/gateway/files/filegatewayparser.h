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

#include "softwarecontainer-common.h"

class FileGatewayParser
{
    LOG_DECLARE_CLASS_CONTEXT("FILP", "File gateway parser");
public:
    struct FileSetting {
        std::string pathInHost;
        std::string pathInContainer;
        bool createSymlinkInContainer;
        bool readOnly;
        std::string envVarName;
        std::string envVarPrefix;
        std::string envVarSuffix;
    };

    ReturnCode parseFileGatewayConfigElement(const json_t *element, FileSetting &setting);
    template<typename T> ReturnCode parseObligatory(const json_t *element, std::string key, T &result);
    template<typename T> ReturnCode parseOptional(const json_t *element, std::string key, T &result);
    template<typename T> ReturnCode assumeFormat(const json_t *element, std::string key, T &result);
};
