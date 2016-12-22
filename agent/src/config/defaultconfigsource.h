
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

#include "softwarecontainer-common.h"
#include "configsource.h"


namespace softwarecontainer {

/**
 * @class DefaultConfigSource
 *
 * @brief Represents the default config values
 */
class DefaultConfigSource : public ConfigSource
{

LOG_DECLARE_CLASS_CONTEXT("CFGD", "Default config source");

public:
    DefaultConfigSource();
    ~DefaultConfigSource() {}

    std::vector<StringConfig> stringConfigs() override;
    std::vector<IntConfig> intConfigs() override;
    std::vector<BoolConfig> boolConfigs() override;

private:
    std::vector<StringConfig> m_stringConfigs;
    std::vector<IntConfig> m_intConfigs;
    std::vector<BoolConfig> m_boolConfigs;
};

} // namespace softwarecontainer