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

#include "functionjob.h"

FunctionJob::FunctionJob(
    SoftwareContainer &sc, std::function<int()> command): JobAbstract(sc)
{
    m_command = command;
}

FunctionJob::~FunctionJob()
{
}

ReturnCode FunctionJob::start()
{
    return m_sc.getContainer()->executeInContainer(
        m_command, &m_pid, m_env, m_stdin[0], m_stdout[1], m_stderr[1]);
}

void FunctionJob::setEnvironmentVariable(
        const std::string &key, const std::string &value)
{
    m_env[key] = value;
}

std::string FunctionJob::toString() const
{
    return logging::StringBuilder() << "SoftwareContainer job. "
                                    << " stdin:" << m_stdin[0]
                                    << " stdout:" << m_stdout[1];
}
