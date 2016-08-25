
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
#include "gateway.h"

/*! Environment Gateway is used to define environment variables to the container
 *  ============================================================================
 *  The environment gateway allows users to specify environment variables that
 *  should be known to the container and all commands and functions running
 *  inside the container.
 *
 *  Required parameters:
 *  * name   string - The name of the environment variable in question
 *  * value  string - The value to attach to the name
 *
 *  Optional parameters:
 *  * append bool   - If the environment variable is already by the environment
 *                    gateway defined append the new value to the value already
 *                    defined. Defaults to false.
 *
 *  Example Configurations
 *  ======================
 *  \code{json}
 *  [
 *      {
 *          "name": "SOME_ENVIRONMENT_VARIABLE",
 *          "value": "SOME_VALUE"
 *      }
 *  ]
 *  \endcode
 *  Note that "value" will be read as a string
 *
 *  There are also the possibility to append to an already defined variable:
 *  \code{json}
 *  [
 *      {
 *          "name": "SOME_ENVIRONMENT_VARIABLE",
 *          "value": "SOME_SUFFIX",
 *          "append": true
 *      }
 *  ]
 *  \endcode
 *
 */
class EnvironmentGateway :
    public Gateway
{
    LOG_DECLARE_CLASS_CONTEXT("Env", "Environment gateway");

public:
    static constexpr const char *ID = "env";

    EnvironmentGateway();
    ~EnvironmentGateway();
    ReturnCode readConfigElement(const json_t *element);
    bool activateGateway();
    bool teardownGateway();

private:
    bool m_enabled = false;
    EnvironmentVariables m_variables;
};
