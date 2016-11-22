
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

#include <glibmm.h>

#include "softwarecontainer-common.h"
#include "configloaderabstractinterface.h"

class FileConfigLoader : public ConfigLoaderAbstractInterface
{

LOG_DECLARE_CLASS_CONTEXT("CFGL", "SoftwareContainer general config loader");

public:
    // Constructor just needs to init parent with the config source string
    FileConfigLoader(const std::string &source) : ConfigLoaderAbstractInterface(source) {}

    std::unique_ptr<Glib::KeyFile> loadConfig() override;
};
