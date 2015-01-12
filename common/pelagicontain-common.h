#pragma once

#include <sys/wait.h>

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

static constexpr const char *APP_BINARY = "/appbin/containedapp";

static constexpr const char *AGENT_OBJECT_PATH = "/com/pelagicore/PelagicontainAgent";
static constexpr const char *AGENT_BUS_NAME = "com.pelagicore.PelagicontainAgent";

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
     * Create a new connection
     */
    sigc::connection &newConnection()
    {
        m_connections.resize(m_connections.size() + 1);
        return m_connections[m_connections.size() - 1];
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
    auto connection = context->signal_child_watch().connect(function, pid);
    connections.newConnection() = connection;
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
            listener( getValue() );
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

using namespace pelagicontain;
using logging::StringBuilder;
