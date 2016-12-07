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

#include "filegatewaysettingstore.h"

FileGatewaySettingStore::FileGatewaySettingStore()
    : m_settings({})
{
}

FileGatewaySettingStore::~FileGatewaySettingStore()
{
}

ReturnCode FileGatewaySettingStore::addSetting(const FileGatewayParser::FileSetting &setting)
{
    auto it = std::find(m_settings.begin(), m_settings.end(), setting);
    if (it != m_settings.end()) {
        if (it->pathInHost != setting.pathInHost) {
            log_error() << "Specifying two files with destination path "
                        << setting.pathInContainer << " but different host paths, "
                        << "this is an error";
            return ReturnCode::FAILURE;
        } else {
            it->readOnly &= setting.readOnly;
            return ReturnCode::SUCCESS;
        }
    }

    m_settings.push_back(setting);
    return ReturnCode::SUCCESS;
}

const std::vector<FileGatewayParser::FileSetting> &FileGatewaySettingStore::getSettings()
{
    return m_settings;
}
