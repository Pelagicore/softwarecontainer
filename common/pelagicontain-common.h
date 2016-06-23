#pragma once

#include <sys/wait.h>
#include <sys/mount.h>

#include <map>
#include <memory>
#include <vector>

#include "pelagicontain-config.h"
#include "pelagicontain-log.h"
#include "pelagicore-common.h"

#include "jsonparser.h"
using pelagicore::JSonElement;

namespace pelagicontain {

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

enum class ReturnCode
{
    FAILURE,
    SUCCESS
};

inline bool isError(ReturnCode code)
{
    return (code != ReturnCode::SUCCESS);
}

inline bool isSuccess(ReturnCode code)
{
    return !isError(code);
}

typedef std::map<std::string, std::string> EnvironmentVariables;

static constexpr const char *AGENT_OBJECT_PATH = "/com/pelagicore/PelagicontainAgent";
static constexpr const char *AGENT_BUS_NAME = "com.pelagicore.PelagicontainAgent";

static constexpr uid_t ROOT_UID = 0;

inline bool isLXC_C_APIEnabled()
{
    return true;
}

inline bool isContainerDeleteEnabled()
{
    return false;
}

/**
 * That class contains references to sigc++ connections and automatically disconnects them on destruction
 */
class SignalConnectionsHandler
{

public:
    /**
     * Add a new connection
     */
    void addConnection(sigc::connection &connection) {
        m_connections.push_back(connection);
    }

    ~SignalConnectionsHandler()
    {
        for (auto &connection : m_connections) {
            connection.disconnect();
        }
    }

private:
    std::vector<sigc::connection> m_connections;

};

inline void addProcessListener(SignalConnectionsHandler &connections, pid_t pid, std::function<void(pid_t, int)> function
            , Glib::RefPtr<Glib::MainContext> context                    // = Glib::MainContext::get_default()
            )
{
    Glib::SignalChildWatch watch = context->signal_child_watch();
    auto connection = watch.connect(function,pid);
    connections.addConnection(connection);
}

inline int waitForProcessTermination(pid_t pid)
{
    int status;
    waitpid(pid, &status, 0);
    return WEXITSTATUS(status);
}

/*
 * Check if path is a directory
 */
bool isDirectory(const std::string &path);
bool isFile(const std::string &path);
bool isSocket(const std::string &path);

bool existsInFileSystem(const std::string &path);

std::string parentPath(const std::string &path);
ReturnCode touch(const std::string &path);
ReturnCode writeToFile(const std::string &path, const std::string &content);
ReturnCode readFromFile(const std::string &path, std::string &content);

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
        auto code = ReturnCode::FAILURE;

        if (!existsInFileSystem(m_path)) {
            log_warning() << "Folder " << m_path << " does no exist";
            return ReturnCode::SUCCESS;
        }

        if (rmdir(m_path.c_str()) == 0) {
            return ReturnCode::SUCCESS;
        } else {
            log_error() << "Can't rmdir " << m_path << " . Error :" << strerror(errno);
            sleep(1);
        }
        return code;
    }

    std::string m_path;
};

class FileCleanUpHandler :
    public CleanUpHandler
{
public:
    FileCleanUpHandler(const std::string &path)
    {
        m_path = path;
    }

    ReturnCode clean() override
    {
        auto code = ReturnCode::FAILURE;

        if (unlink(m_path.c_str()) == 0) {
            code = ReturnCode::SUCCESS;
        } else {
            log_error() << "Can't delete " << m_path << " . Error :" << strerror(errno);
        }

        return code;
    }

    std::string m_path;
};

class MountCleanUpHandler :
    public CleanUpHandler
{
public:
    MountCleanUpHandler(const std::string &path)
    {
        m_path = path;
    }

    ReturnCode clean() override
    {
        auto code = ReturnCode::FAILURE;

        // Lazy unmount. Should be the equivalent of the "umount -l" command. The unmount will actually happen when no-one uses the resource anymore.
        if (umount(m_path.c_str()) == 0) {
            code = ReturnCode::SUCCESS;
            log_debug() << "Unmounted " << m_path;
        } else {
            log_warn() << "Can't unmount " << m_path << " . Error :" << strerror(errno) << ". Trying to force umount";
            if (umount2(m_path.c_str(), MNT_FORCE) == 0) {
                log_warn() << "Can't force unmount " << m_path << " . Error :" << strerror(errno) << ". Trying to force umount";
            }
        }

        return code;
    }

    std::string m_path;
};


class FileToolkitWithUndo
{
    LOG_DECLARE_CLASS_CONTEXT("CLEA", "File toolkit");

public:
    ~FileToolkitWithUndo()
    {
        // Clean up all created directories, files, and mount points
        for (auto it = m_cleanupHandlers.rbegin(); it != m_cleanupHandlers.rend(); ++it) {
            if (!isSuccess((*it)->clean())) {
                log_error() << "Error";
            }
            delete *it;
        }
    }

    ReturnCode createParentDirectory(const std::string &path)
    {
        log_debug() << path;
        auto parent = parentPath(path);
        if (!isDirectory(parent)) {
            createDirectory(parent);
        }
        return ReturnCode::SUCCESS;
    }

    /**
     * Create a directory, and if successful append it to a list of dirs
     * to be deleted in the dtor. Since nestled dirs will need to be
     * deleted in reverse order to creation insert to the beginning of
     * the list.
     */
    ReturnCode createDirectory(const std::string &path);

    ReturnCode bindMount(const std::string &src, const std::string &dst, bool readOnly)
    {

        int flags = MS_BIND;

        if (readOnly) {
            flags |= MS_RDONLY;
        }

        log_debug() << "Mounting " << (readOnly ? " readonly " : "read/write ") << src << " in " << dst << " / flags: " << flags;

        int mountRes = mount(src.c_str(), // source
                    dst.c_str(),          // target
                    "",                   // fstype
                    flags,              // flags
                    nullptr);                // data

        auto result = ReturnCode::FAILURE;

        if (mountRes == 0) {
            // Success
            m_cleanupHandlers.push_back(new MountCleanUpHandler(dst));

            log_verbose() << "Mounted folder " << src << " in " << dst;
            result = ReturnCode::SUCCESS;
        } else {
            // Failure
            log_error() << "Could not mount into container: src=" << src << " , dst=" << dst << " err=" << strerror(errno);
        }

        return result;
    }

    ReturnCode createSharedMountPoint(const std::string &path)
    {
        // MS_MGC_VAL |
        auto mountRes = mount(path.c_str(), path.c_str(), "", MS_BIND, nullptr);
        assert(mountRes == 0);
        mountRes = mount(path.c_str(), path.c_str(), "", MS_UNBINDABLE, nullptr);
        assert(mountRes == 0);
        mountRes = mount(path.c_str(), path.c_str(), "", MS_SHARED, nullptr);
        assert(mountRes == 0);
        m_cleanupHandlers.push_back(new MountCleanUpHandler(path));

        return ReturnCode::SUCCESS;
    }

    ReturnCode writeToFile(const std::string &path, const std::string &content)
    {
        auto ret = pelagicontain::writeToFile(path, content);
        if (isError(ret)) {
            return ret;
        }
        m_cleanupHandlers.push_back(new FileCleanUpHandler(path));
        return ReturnCode::SUCCESS;
    }

    ReturnCode createSymLink(const std::string &source, const std::string &destination)
    {
        log_debug() << "creating symlink " << source << " pointing to " << destination;

        createDirectory(parentPath(source));

        if (symlink(destination.c_str(), source.c_str()) == 0) {
            m_cleanupHandlers.push_back(new FileCleanUpHandler(source));
        } else {
            log_error() << "Error creating symlink " << destination << " pointing to " << source << ". Error: " << strerror(errno);
            return ReturnCode::FAILURE;
        }

        return ReturnCode::SUCCESS;
    }

protected:
    std::vector<CleanUpHandler *> m_cleanupHandlers;

};

}

using namespace pelagicontain;
using logging::StringBuilder;
