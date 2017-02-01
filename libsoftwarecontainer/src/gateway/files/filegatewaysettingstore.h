/*
 * Copyright (C) 2016-2017 Pelagicore AB
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

#include "filegatewayparser.h"

namespace softwarecontainer {

class FileGatewaySettingStore
{
    LOG_DECLARE_CLASS_CONTEXT("FGWR", "File Gateway Rule Engine");

public:
    /*
     * @brief Construct a new empty setting store
     */
    FileGatewaySettingStore();

    /*
     * @brief Destruct the setting store
     */
    ~FileGatewaySettingStore();

    /*
     * @brief Add a new setting to the store
     *
     * @param setting the file setting to be added
     * @return false if the setting conflicts with any existing settings
     * @return true otherwise
     */
    bool addSetting(const FileGatewayParser::FileSetting &setting);

    /*
     * @brief Get the list of settings
     *
     * @return A list of well-formed file settings, ready to be applied
     */
    const std::vector<FileGatewayParser::FileSetting> &getSettings();

private:
    // The internal storage of settings
    std::vector<FileGatewayParser::FileSetting> m_settings;
};

} // namespace softwarecontainer
