
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

/*
 * Abstract interface to a concrete loader.
 *
 * The concrete loader will either be a production loader that loads a config
 * from a file on the system, or a test loader that loads a config from a string.
 */
class ConfigLoaderAbstractInterface
{
public:
    // Enforce inheriting classes to initialize config source member
    ConfigLoaderAbstractInterface() = delete;
    ConfigLoaderAbstractInterface(const std::string &source) : m_source(source) {}

    virtual ~ConfigLoaderAbstractInterface() {}

    virtual std::unique_ptr<Glib::KeyFile> loadConfig() = 0;

protected:
    // A string with the source of the config, e.g. a path or config string.
    std::string m_source;
};
