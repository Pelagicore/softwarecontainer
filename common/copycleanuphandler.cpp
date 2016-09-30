#include "copycleanuphandler.h"

#include "recursivecopy.h"

CopyCleanupHandler::CopyCleanupHandler(std::string src, std::string dst)
{
    m_src = src;
    m_dst = dst;
}

ReturnCode CopyCleanupHandler::clean()
{
    return RecursiveCopy::getInstance().copy(m_src, m_dst);
}
