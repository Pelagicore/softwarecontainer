#ifndef COPYCLEANUPHANDLER_H
#define COPYCLEANUPHANDLER_H

#include <cleanuphandler.h>

class CopyCleanupHandler : public CleanUpHandler
{
public:
    CopyCleanupHandler(std::string src, std::string dst);

    ReturnCode clean() override;
private:
    std::string m_src;
    std::string m_dst;

};

#endif // COPYCLEANUPHANDLER_H
