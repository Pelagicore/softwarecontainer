
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

#include "softwarecontainer-config.h"
#include "softwarecontainer-log.h"

namespace softwarecontainer {

typedef uint32_t ContainerID;

static constexpr ContainerID INVALID_CONTAINER_ID = -1;
static constexpr pid_t INVALID_PID = -1;
static constexpr int INVALID_FD = -1;

typedef std::map<std::string, std::string> GatewayConfiguration;

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

bool operator&& (ReturnCode lhs, ReturnCode rhs);


inline ReturnCode bool2ReturnCode(bool b)
{
    return b ? ReturnCode::SUCCESS : ReturnCode::FAILURE;
}

inline bool isError(ReturnCode code)
{
    return (code != ReturnCode::SUCCESS);
}

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
        SignalConnectionsHandler &connections
        , pid_t pid
        , std::function<void(pid_t, int)> function
        , Glib::RefPtr<Glib::MainContext> context)
{
    Glib::SignalChildWatch watch = context->signal_child_watch();
    auto connection = watch.connect(function,pid);
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

class CleanUpHandler
{
protected:
    LOG_DECLARE_CLASS_CONTEXT("CLEA", "Cleanup handler");
public:
    virtual ~CleanUpHandler()
    {
    }
    virtual ReturnCode clean() = 0;
};

class DirectoryCleanUpHandler :
    public CleanUpHandler
{
public:
    DirectoryCleanUpHandler(const std::string &path)
    {
        m_path = path;
    }

    ReturnCode clean() override
    {
        if (!existsInFileSystem(m_path)) {
            log_warning() << "Folder " << m_path << " does not exist";
            return ReturnCode::SUCCESS;
        }

        if (rmdir(m_path.c_str()) == 0) {
            log_debug() << "rmdir'd " << m_path;
            return ReturnCode::SUCCESS;
        } else {
            log_error() << "Can't rmdir " << m_path << " . Error :" << strerror(errno);
            return ReturnCode::FAILURE;
        }
    }

    std::string m_path;
};

class FileCleanUpHandler :
    public CleanUpHandler
{
public:
    FileCleanUpHandler(const std::string &path);

    ReturnCode clean() override;

    std::string m_path;
};

class MountCleanUpHandler :
    public CleanUpHandler
{
public:
    MountCleanUpHandler(const std::string &path);

    ReturnCode clean() override;
    std::string m_path;
};

class FileToolkitWithUndo
{
    LOG_DECLARE_CLASS_CONTEXT("CLEA", "File toolkit");

public:
    ~FileToolkitWithUndo();

    /**
     * @brief createParentDirectory Recursively tries to create the directory pointed to by path.
     * @param path The directory path to be created.
     * @return ReturnCode::SUCCESS on success, ReturnCode::FAILURE on failure
     */
    ReturnCode createParentDirectory(const std::string &path);

    /**
     * @brief createDirectory Create a directory, and if successful append it
     *  to a list of dirs to be deleted in the dtor. Since nestled dirs will
     *  need to be deleted in reverse order to creation insert to the beginning
     *  of the list.
     * @param path Path of directory to be created
     * @return ReturnCode::SUCCESS on success, ReturnCode::FAILURE on failure
     */
    ReturnCode createDirectory(const std::string &path);

    /**
     * @brief bindMount Bind mount a src directory to another position dst.
     * @param src Path to mount from
     * @param dst Path to mount to
     * @param readOnly Make the bind mount destination read only
     * @param enableWriteBuffer Enable write buffers on the bind mount.
     * @return ReturnCode::SUCCESS on success, ReturnCode::FAILURE on failure
     */
    ReturnCode bindMount(const std::string &src, const std::string &dst, bool readOnly, bool enableWriteBuffer=false);

    /**
     * @brief overlayMount Mount a directory with an overlay on top of it. An overlay protects
     *  the lower filesystem from writes by writing to the upper file system through the work
     *  directory.
     * @param lower The lower file system, this will be read only.
     * @param upper The upper file system, this can be a tmpfs/ramfs of some kind. This is where
     *  final writes wind up
     * @param work This is a work directory, preferably a tmpfs/ramfs of some kind. This is where
     *  writes wind up temporarily.
     * @param dst Where the overlay filesystem will be mounted.
     * @return ReturnCode::SUCCESS on success, ReturnCode::FAILURE on failure
     */
    ReturnCode overlayMount(
            const std::string &lower
            , const std::string &upper
            , const std::string &work
            , const std::string &dst);

    /**
     * @brief createSharedMountPoint Make the mount point shared, ie new mount points created in
     *  one bind mount will also be created in the other mount point.
     * @param path The mount path to make shared.
     * @return ReturnCode::SUCCESS on success, ReturnCode::FAILURE on failure
     */
    ReturnCode createSharedMountPoint(const std::string &path);

    ReturnCode writeToFile(const std::string &path, const std::string &content);

    ReturnCode createSymLink(const std::string &source, const std::string &destination);

    /**
     * @brief tempDir Creates a temporary directory at templatePath.
     * @warning The temporary path will be destroyed when the instance of FileToolkitWithUndo
     *  is destroyed.
     * @param templ a template Path used to create the path of the temporary directory, including
     *  XXXXXX which will be replaced with a unique ID for the temporary directory
     * @return A string path pointing to the newly creted temporary directory.
     */
    std::string tempDir(std::string templatePath);
protected:
    /**
     * @brief m_cleanupHandlers A vector of cleanupHandlers added during the lifetime of the
     *  FileToolKitWithUndo that will be run from the destructor.
     */
    std::vector<CleanUpHandler *> m_cleanupHandlers;
};

}

using namespace softwarecontainer;
using logging::StringBuilder;
