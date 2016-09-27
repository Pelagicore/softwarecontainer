#pragma once

#include "softwarecontainer-common.h"
#include "softwarecontainer-log.h"

namespace softwarecontainer {

class CleanUpHandler
{
protected:
    LOG_DECLARE_CLASS_CONTEXT("CLEA", "Cleanup handler");
public:
    virtual ~CleanUpHandler()
    {
    }
    virtual ReturnCode clean() = 0;
};

}
