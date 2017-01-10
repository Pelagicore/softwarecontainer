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

#include "jobabstract.h"

class ContainerAbstractInterface;
namespace softwarecontainer {

/**
 * Run a C++ function lambda inside a SoftwareContainer. 
 */
class FunctionJob :
    public JobAbstract
{

public:
    static constexpr int UNASSIGNED_STREAM = -1;

    FunctionJob(ExecutablePtr executable, std::function<int()> fun);
    virtual ~FunctionJob();

    ReturnCode start();
    void setEnvironmentVariable(const std::string &key, const std::string &value);
    std::string toString() const;

private:
    std::function<int()> m_command;
};

} // namespace softwarecontainer
