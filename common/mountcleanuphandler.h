#pragma once

#include <cleanuphandler.h>

class MountCleanUpHandler :
    public CleanUpHandler
{
public:
    MountCleanUpHandler(const std::string &path);

    ReturnCode clean() override;
    std::string m_path;
};
