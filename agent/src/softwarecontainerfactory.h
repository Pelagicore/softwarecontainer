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
 * @file softwarecontainerfactory.h
 * @brief Contains the softwarecontainer::SoftwareContainerFactory class
 */
#pragma once

#include "softwarecontainer.h"
#include "config/softwarecontainerconfig.h"
#include "softwarecontainer-common.h"

namespace softwarecontainer {

/*
 * This class creates containers.
 */

class SoftwareContainerFactory {
public:
    virtual ~SoftwareContainerFactory() {};

    virtual std::shared_ptr<SoftwareContainerAbstractInterface>
            createContainer(const ContainerID id,
                            std::unique_ptr<const SoftwareContainerConfig> config);
};

} //namespace

