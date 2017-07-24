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

#include "containerutilityinterface.h"

#include "softwarecontainer-common.h"
#include "createdir.h"

#include <lxc/lxccontainer.h>
#include <regex>

namespace softwarecontainer {
ContainerUtilityInterface::ContainerUtilityInterface(std::shared_ptr<Config> config)
    : m_config(std::move(config))
{
}

void ContainerUtilityInterface::removeOldContainers()
{
    char **containerNames = nullptr;
    struct lxc_container **containerList = nullptr;

    const char *basePath = lxc_get_global_config_item("lxc.lxcpath");
    auto num = list_all_containers(basePath, &containerNames, &containerList);

    if (-1 == num) {
        log_error() << "An error occurred while trying to get container list";
        throw ContainerUtilityInterfaceError("An error occurred while trying to get container list");
    } else if (0 == num) {
        delete containerNames;
        delete containerList;
        return;
    }

    // Convert the list/names to a map for convenience
    std::map<std::string, struct lxc_container *> containerMap;
    for(int i=0; i<num; i++) {
        std::string name = std::string(containerNames[i]);
        struct lxc_container *container = containerList[i];

        containerMap[name] = container;
        delete containerNames[i]; // We don't need this anymore
    }
    delete containerNames; // We don't need this anymore

    for(const auto value : containerMap) {
        std::string containerName = value.first;
        struct lxc_container *container = value.second;

        if (!std::regex_match(containerName, std::regex("SC-.*"))) {
            log_debug() << "Found non-SC container " << containerName;
            continue;
        }

        log_debug() << "Deprecated container named " << containerName << " will be deleted";

        if (container->is_running(container)) {
            bool success = container->stop(container);
            if (!success) {
                std::string errorMsg = "Unable to stop deprecated container " + containerName;
                throw ContainerUtilityInterfaceError(errorMsg);
            }
        }

        bool success = container->destroy(container);
        if (!success) {
            std::string errorMsg = "Unable to destroy deprecated container " + containerName;
            throw ContainerUtilityInterfaceError(errorMsg);
        }

        log_debug() << "Deprecated container " << containerName << " is successfully destroyed";
        delete value.second; // Remove the lxc_container struct that we allocated
    }
}

void ContainerUtilityInterface::checkWorkspace()
{
    const std::string rootDir = m_config->getStringValue("SoftwareContainer", "shared-mounts-dir");
    if (!isDirectory(rootDir)) {
        log_debug() << "Container root " << rootDir << " does not exist, trying to create";
        std::unique_ptr<CreateDir> createDirInstance(new CreateDir());
        if(!createDirInstance->createDirectory(rootDir)) {
            std::string message = "Failed to create container root directory";
            log_error() << message;
            throw SoftwareContainerError(message);
        }

        m_createDirList.push_back(std::move(createDirInstance));
    }
}


} //namespace
