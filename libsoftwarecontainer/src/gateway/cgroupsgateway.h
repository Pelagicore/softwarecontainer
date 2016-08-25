
/*
 * Copyright (C) 2016 Pelagicore AB
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


#pragma once

#include "gateway.h"

/*! The cgroups gateway sets cgroups related settings for the container.
 *
 * The gateway configuration contains settings as key/value pairs and the setting
 * key and value will be applied using 'lxc-cgroup' as they are written in the
 * gateway config (the 'lxc.cgroup' prefix is added to all keys by the gateway).
 *
 * No syntax or other checks for correctness is performed on the key/value pairs,
 * see the lxc.container.conf man page for more details about legal settings.
 *
 * JSON format used in gateway config (as passed to setConfig():
 * \code{.js}
 * [
 *   {
 *     "setting": "memory.limit_in_bytes",
 *     "value": "128M"
 *   },
 *   {
 *     "setting": "cpu.shares",
 *     "value":  "256"
 *   }
 * ]
 * \endcode
 *
 * The root object is an array of seting key/value pair objects. Each key/value pair
 * must have the 'setting' and 'value' defined. With the above example config the calls
 * to lxc-cgroup would set the following:
 * - lxc.cgroup.memory.limit_in_bytes to '128M'
 * - lxc.cgroup.cpu.shares to '256'
 *
 * It is an error to prepend the 'lxc.cgroup' prefix to settings in the config.
 */

class CgroupsGateway: public Gateway
{
    LOG_DECLARE_CLASS_CONTEXT("CGRO", "Cgroups gateway");

public:
    static constexpr const char *ID = "cgroups";

    CgroupsGateway();
    ~CgroupsGateway() { }

    ReturnCode readConfigElement(const json_t *element) override;
    bool activateGateway() override;
    bool teardownGateway() override;

private:
    std::map<std::string, std::string> m_settings;
};
