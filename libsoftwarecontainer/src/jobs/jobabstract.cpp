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

#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include "jobabstract.h"

namespace softwarecontainer {

JobAbstract::JobAbstract(ExecutablePtr &executable) :
    m_executable(executable)
{
}

JobAbstract::~JobAbstract()
{
}

void JobAbstract::captureStdin()
{
    pipe(m_stdin);
}

void JobAbstract::setOutputFile(const std::string &path)
{
    mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
    m_stdout[1] = m_stderr[1] = ::open(path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, mode);
    log_debug() << "stdout/stderr redirected to " << path << " fd:" << m_stdout[0];
}

void JobAbstract::captureStdout()
{
    pipe(m_stdout);
}

void JobAbstract::captureStderr()
{
    pipe(m_stderr);
}

int JobAbstract::wait()
{
    return waitForProcessTermination(m_pid);
}

int JobAbstract::stdout()
{
    return m_stdout[0];
}

int JobAbstract::stderr()
{
    return m_stderr[0];
}

int JobAbstract::stdin()
{
    return m_stdin[1];
}

pid_t JobAbstract::pid()
{
    return m_pid;
}

/**
 * That method always returns true as soon as the start() method has been called, even if the command fails to start,
 * since we don't know if the exec() occurring after the fork into the container actually succeeds...
 */
bool JobAbstract::isRunning()
{
    // TODO : find a way to test whether the exec() which occurs in the container succeeded
    return (m_pid != 0);
}

void JobAbstract::setEnvironmentVariable(const std::string &key, const std::string &value)
{
    m_env[key] = value;
}

void JobAbstract::setEnvironmentVariables(const EnvironmentVariables &env)
{
    m_env = env;
}

} // namespace softwarecontainer
