#pragma once

#include <cleanuphandler.h>

class FileCleanUpHandler :
    public CleanUpHandler
{
public:
    FileCleanUpHandler(const std::string &path);

    ReturnCode clean() override;

    std::string m_path;
};
