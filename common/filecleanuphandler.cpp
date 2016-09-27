#include "filecleanuphandler.h"

FileCleanUpHandler::FileCleanUpHandler(const std::string &path)
{
    m_path = path;
}

ReturnCode FileCleanUpHandler::clean()
{
    if (unlink(m_path.c_str()) == 0) {
        log_debug() << "Unlinked " << m_path;
        return ReturnCode::SUCCESS;
    } else {
        log_error() << "Can't delete " << m_path << " . Error :" << strerror(errno);
        return ReturnCode::FAILURE;
    }
}
