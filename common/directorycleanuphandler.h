#pragma once

#include <cleanuphandler.h>

class DirectoryCleanUpHandler :
    public CleanUpHandler
{
public:
    DirectoryCleanUpHandler(const std::string &path);

    ReturnCode clean() override;

    std::string m_path;
};
