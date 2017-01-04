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
#include "jsonparser.h"

namespace softwarecontainer {

class EnvironmentGatewayParser
{
    LOG_DECLARE_CLASS_CONTEXT("ENVP", "Environment gateway parser");

public:
    typedef std::pair<std::string, std::string> EnvironmentVariable;
    ReturnCode parseEnvironmentGatewayConfigElement(const json_t *element,
                                                    EnvironmentVariable &result,
                                                    const EnvironmentVariables &store);
private:
    ReturnCode requireNonEmptyKeyValue(const json_t *element,
                                       const std::string key,
                                       std::string &result);
};

} // namespace softwarecontainer
