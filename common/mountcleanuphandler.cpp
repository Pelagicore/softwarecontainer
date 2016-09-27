#include "mountcleanuphandler.h"


MountCleanUpHandler::MountCleanUpHandler(const std::string &path)
{
    m_path = path;
}

ReturnCode MountCleanUpHandler::clean()
{
    // Lazy unmount. Should be the equivalent of the "umount -l" command.
    if (umount2(m_path.c_str(), MNT_DETACH) == 0) {
        log_debug() << "Unmounted " << m_path;
        return ReturnCode::SUCCESS;
    } else {
        log_warn() << "Can't unmount " << m_path << " . Error :" << strerror(errno) << ". Trying to force umount";
        if (umount2(m_path.c_str(), MNT_FORCE) != 0) {
            log_warn() << "Can't force unmount " << m_path << " . Error :" << strerror(errno);
            return ReturnCode::FAILURE;
        }
        log_debug() << "Managed to force unmount " << m_path;
        return ReturnCode::SUCCESS;
    }
}

