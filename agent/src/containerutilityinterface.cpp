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

#include <lxc/lxccontainer.h>

namespace softwarecontainer {

void ContainerUtilityInterface::removeOldContainers()
{
    char **containerNames = nullptr;
    struct lxc_container **containerList = nullptr;

    const char *basePath = lxc_get_global_config_item("lxc.lxcpath");
    auto num = list_all_containers(basePath, &containerNames, &containerList);

    if (0 == num) {
        // there are no container residue. No need to run more.
        delete containerList;
        delete containerNames;
        return;
    } else if (-1 == num) {
        log_error() << "An error is occurred while trying to get deprecated container list";
        throw ContainerUtilityInterfaceError("An error is occurred while trying to get container list");
    }

    log_warning() << num << " unused deprecated containers found";
    for (auto i = 0; i < num; i++) {
        struct lxc_container *container = containerList[i];
        log_debug() << "Deprecated container named " << containerNames[i] << " will be deleted";

        if (container->is_running(container)) {
            bool success = container->stop(container);
            if (!success) {
                std::string errorMsg = "Unable to stop deprecated container " +
                                       std::string(containerNames[i]);
                throw ContainerUtilityInterfaceError(errorMsg);
            }
        }

        bool success = container->destroy(container);
        if (!success) {
            std::string errorMsg = "Unable to destroy deprecated container " +
                                   std::string(containerNames[i]);
            throw ContainerUtilityInterfaceError(errorMsg);
        }

        log_debug() << "Deprecated container " << containerNames[i] << " is successfully destroyed";
        delete container;
        delete containerNames[i];
    }
    delete containerList;
    delete containerNames;
}

} //namespace
