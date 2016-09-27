#include "directorycleanuphandler.h"

DirectoryCleanUpHandler::DirectoryCleanUpHandler(const std::string &path)
{
    m_path = path;
}

ReturnCode DirectoryCleanUpHandler::clean()
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
