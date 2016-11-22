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

#include <sys/wait.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <map>
#include <memory>
#include <vector>

#include <jansson.h>

#include "softwarecontainer-log.h"

namespace softwarecontainer {

typedef int32_t ContainerID;

static constexpr ContainerID INVALID_CONTAINER_ID = -1;
static constexpr pid_t INVALID_PID = -1;
static constexpr int INVALID_FD = -1;

enum class ContainerState
{
    CREATED,
    PRELOADED,
    READY,
    //    RUNNING,
    TERMINATED
};

/**
 * @brief The ReturnCode enum contains common return values.
 */
enum class ReturnCode
{
    FAILURE,
    SUCCESS
};

/**
 * @brief bool2ReturnCode Convert bool to ReturnCode
 * @param b boolean value to be converted
 * @return ReturnCode::SUCCESS if b is true
 * @return ReturnCode::FAILURE if b is false
 */
inline ReturnCode bool2ReturnCode(bool b)
{
    return b ? ReturnCode::SUCCESS : ReturnCode::FAILURE;
}

/**
 * @brief isError Check if ReturnCode indicates Error or not
 * @param code the ReturnCode to check
 * @return true if ReturnCode::FAILURE is indicated
 */
inline bool isError(ReturnCode code)
{
    return (code != ReturnCode::SUCCESS);
}

/**
 * @brief isSuccess Check if ReturnCode indicates Success or not
 * @param code the ReturnCode to check
 * @return true if ReturnCode::SUCCESS is indicated
 */
inline bool isSuccess(ReturnCode code)
{
    return !isError(code);
}

typedef std::map<std::string, std::string> EnvironmentVariables;

static constexpr const char *AGENT_OBJECT_PATH = "/com/pelagicore/SoftwareContainerAgent";
static constexpr const char *AGENT_BUS_NAME = "com.pelagicore.SoftwareContainerAgent";

static constexpr uid_t ROOT_UID = 0;

/**
 * @brief The SignalConnectionsHandler class contains references to sigc++ connections and
 * automatically disconnects them on destruction.
 */
class SignalConnectionsHandler
{

public:
    /**
     * Add a new connection
     */
    void addConnection(sigc::connection &connection);

    ~SignalConnectionsHandler();

private:
    std::vector<sigc::connection> m_connections;

};

/**
 * @brief addProcessListener Adds a glib child watch for a process.
 * @warning This is not thread safe!
 * @param connections Add the signal to this list of connections
 * @param pid The pid to watch for.
 * @param function A lambda/function pointer to run when the signal is sent for a process.
 * @param context glib context to attach the SignalChildWatch to.
 */
inline void addProcessListener(
    SignalConnectionsHandler &connections,
    pid_t pid,
    std::function<void(pid_t, int)> function,
    Glib::RefPtr<Glib::MainContext> context)
{
    Glib::SignalChildWatch watch = context->signal_child_watch();
    auto connection = watch.connect(function, pid);
    connections.addConnection(connection);
}

/**
 * @brief waitForProcessTermination Waits for a process to terminate and then returns the status
 * of the process terminating.
 * @param pid the process id to wait for.
 * @return Process termination status.
 */
inline int waitForProcessTermination(pid_t pid)
{
    int status = 0;
    waitpid(pid, &status, 0);
    return WEXITSTATUS(status);
}

/**
 * @brief isDirectory Check if path is a directory
 * @param path Path to check
 * @return true/false
 */
bool isDirectory(const std::string &path);

/**
 * @brief isDirectoryEmpty Check if path is empty
 * @param path The path to check
 * @return false if the directory is not empty
 * @return false if the directory does not exist
 * @return true if the directory is empty
 */
bool isDirectoryEmpty(const std::string &path);

/**
 * @brief isFile Check if path is a file
 * @param path Path to check
 * @return true/false
 */
bool isFile(const std::string &path);

/**
 * @brief isPipe Check if path is a pipe
 * @param path Path to check
 * @return true/false
 */
bool isPipe(const std::string &path);

/**
 * @brief isSocket Check if path is a socket
 * @param path Path to check
 * @return true/false
 */
bool isSocket(const std::string &path);

/**
 * @brief existsInFileSystem Check if path exists
 * @param path Path to check
 * @return true/false
 */
bool existsInFileSystem(const std::string &path);

std::string parentPath(const std::string &path);
ReturnCode touch(const std::string &path);
ReturnCode writeToFile(const std::string &path, const std::string &content);
ReturnCode readFromFile(const std::string &path, std::string &content);
bool parseInt(const char *args, int *result);

template<typename Type>
class ObservableProperty
{
public:
    typedef std::function<void (const Type &)> Listener;

    ObservableProperty(Type &value) :
        m_value(value)
    {
    }

    void addListener(Listener listener)
    {
        m_listeners.push_back(listener);
    }

    operator const Type &() {
        return m_value;
    }

protected:
    std::vector<Listener> m_listeners;

private:
    const Type &m_value;

};

template<typename Type>
class ObservableWritableProperty :
    public ObservableProperty<Type>
{
public:
    ObservableWritableProperty(Type value) :
        ObservableProperty<Type>(m_value), m_value(value)
    {
    }

    ObservableWritableProperty() :
        ObservableProperty<Type>(m_value)
    {
    }

    void setValueNotify(Type value)
    {
        m_value = value;
        for (auto &listener : ObservableProperty<Type>::m_listeners) {
            listener(getValue());
        }
    }

    const Type &getValue() const
    {
        return m_value;
    }

    ObservableWritableProperty &operator=(const Type &type)
    {
        m_value = type;
        return *this;
    }


private:
    Type m_value;

};

}

using namespace softwarecontainer;
using logging::StringBuilder;
