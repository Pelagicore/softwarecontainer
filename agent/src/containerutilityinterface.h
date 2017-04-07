/*
 * Copyright (C) 2017 Pelagicore AB
 *
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
 * WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR
 * BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES
 * OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
 * ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 *
 * For further information see LICENSE
 */

/**
 * @file containerutilityinterface.h
 * @brief Contains the softwarecontainer::ContainerUtilityAbstractInterface class
 */
#pragma once

#include "softwarecontainer-log.h"
#include "softwarecontainererror.h"
#include "config/config.h"
#include "filetoolkitwithundo.h"

#include <memory>

namespace softwarecontainer {

/**
 * @brief An error occured in ContainerUtilityInterface
 *
 * This exception should be used if an internal error occurs in the
 * ContainerUtilityInterface.
 *
 */
class ContainerUtilityInterfaceError : public SoftwareContainerError
{
public:
    ContainerUtilityInterfaceError():
        m_message("SoftwareContainer error")
    {
    }

    ContainerUtilityInterfaceError(const std::string &message):
        m_message(message)
    {
    }

    virtual const char *what() const throw()
    {
        return m_message.c_str();
    }

protected:
    std::string m_message;
};

/*
 * This class contains utility functions
 */

class ContainerUtilityInterface : private FileToolkitWithUndo
{
    LOG_DECLARE_CLASS_CONTEXT("CUI", "Container Utility Interface");
public:
    ContainerUtilityInterface(std::shared_ptr<Config> config);
    virtual ~ContainerUtilityInterface() {}

    /**
     * @brief This method cleans unused old containers before agent starts up
     */
    void removeOldContainers(void);
    /**
     * @brief Check that the workspace exists.
     */
    void checkWorkspace(void);

private:
    std::shared_ptr<Config> m_config;
};

} //namespace

