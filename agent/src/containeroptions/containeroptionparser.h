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

#pragma once

#include "jansson.h"
#include <memory>

#include "softwarecontainer-common.h"
#include "softwarecontainererror.h"

#include "containeroptions/dynamiccontaineroptions.h"

namespace softwarecontainer {

class ContainerOptionParseError : public SoftwareContainerError
{
public:
    ContainerOptionParseError() :
        SoftwareContainerError("Container config parse error")
    {
    }

    ContainerOptionParseError(const std::string &message) :
        SoftwareContainerError(message)
    {
    }
};

/*
 * @brief a parser for dynamic container configurations.
 */
class ContainerOptionParser
{
    LOG_DECLARE_CLASS_CONTEXT("CCPA", "ContainerOptionParser");

public:
    ContainerOptionParser();

    /**
     * @brief Parse config needed for starting up the container in a correct manner.
     *
     * This reads a config string into json format, makes sure it is formatted
     * correctly, and then calls readConfigElement for its elements.
     *
     * @param config a configuration string
     * @return a DynamicContainerOptions object with the given settings set.
     * @throws a ContainerOptionParserError in case of bad input
     */
    std::unique_ptr<DynamicContainerOptions> parse(const std::string &config);

private:
    /**
     * @brief Tries to read a config in json format for a container
     *
     * One can pass options to the agent when creating containers, options that
     * are not gateway specific. These options will be read by this function.
     * This will also set the corresponding field in the given ContainerOptions struct
     *
     * @param element a config snippet in json format
     * @throws ContainerOptionParseError on failure.
     */
    void readConfigElement(const json_t *element);

    std::unique_ptr<DynamicContainerOptions> m_options;

    static constexpr int DEFAULT_TEMPORARY_FILESYSTEM_SIZE = 100 * 1024 * 1024;
};

} // namespace softwarecontainer
