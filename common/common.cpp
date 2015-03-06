#include <string.h>
#include <string>
#include <sys/stat.h>
#include <fcntl.h>
#include <fstream>
#include "pelagicontain-common.h"

namespace pelagicontain {

LOG_DECLARE_DEFAULT_CONTEXT(defaultLogContext, "MAIN", "Main context");

struct stat getStat(const std::string &path)
{
    struct stat st = {};
    if (stat(path.c_str(), &st) == 0) {
        return st;
    }
    return st;
}

bool isDirectory(const std::string &path)
{
    return S_ISDIR(getStat(path).st_mode);
}

bool isFile(const std::string &path)
{
    return S_ISREG(getStat(path).st_mode);
}

bool isSocket(const std::string &path)
{
    return S_ISSOCK(getStat(path).st_mode);
}

bool existsInFileSystem(const std::string &path)
{
    return (getStat(path).st_mode != 0);
}

std::string parentPath(const std::string &path)
{
    static constexpr const char *separator = "/";
    auto pos = path.rfind(separator);
    if (pos == std::string::npos) {
        pos = strlen(separator);
    }
    std::string parentPath = path.substr(0, pos - strlen(separator) + 1);
    return parentPath;
}

ReturnCode touch(const std::string &path)
{
    auto fd = open(path.c_str(), O_WRONLY | O_CREAT | O_NOCTTY | O_NONBLOCK | O_LARGEFILE, 0666);
    if (fd != -1) {
        close(fd);
        return ReturnCode::FAILURE;
    }
    return ReturnCode::SUCCESS;

}

ReturnCode writeToFile(const std::string &path, const std::string &content)
{
    log_debug() << "writing to " << path << " : " << content;
    std::ofstream out(path);     // TODO : error checking
    out << content;
    return ReturnCode::SUCCESS;
}

ReturnCode readFromFile(const std::string &path, std::string &content)
{
    std::ifstream t(path);
    std::stringstream buffer;
    buffer << t.rdbuf();
    content = buffer.str();
    return ReturnCode::SUCCESS;
}

}
