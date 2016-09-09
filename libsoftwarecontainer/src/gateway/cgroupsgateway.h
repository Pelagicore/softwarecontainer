
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

/*!
 * The cgroups gateway sets cgroups related settings for the container.
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
