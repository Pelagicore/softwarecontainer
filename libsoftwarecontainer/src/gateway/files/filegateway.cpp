
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


#include "filegateway.h"

FileGateway::FileGateway()
    : Gateway(ID)
    , m_settings({})
{
}

ReturnCode FileGateway::readConfigElement(const json_t *element)
{
    FileGatewayParser parser;
    FileGatewayParser::FileSetting setting;

    if (isError(parser.parseConfigElement(element, setting))) {
        return ReturnCode::FAILURE;
    }

    if (isError(parser.matchEntry(setting, m_settings))) {
        return ReturnCode::FAILURE;
    }

    return ReturnCode::SUCCESS;
}

bool FileGateway::activateGateway()
{
    if (m_settings.size() > 0) {
        for (FileGatewayParser::FileSetting &setting : m_settings) {
            if (!bindMount(setting)) {
                log_error() << "Bind mount failed";
                return false;
            }
        }
        return true;
    }

    return false;
}

bool FileGateway::bindMount(const FileGatewayParser::FileSetting &setting)
{
    if (isDirectory(setting.pathInHost)) {
        if (isError(getContainer()->bindMountFolderInContainer(setting.pathInHost,
                                                    setting.pathInContainer,
                                                    setting.readOnly))) {
            log_error() << "Could not bind mount folder into container";
            return false;
        }
    } else {
        if (isError(getContainer()->bindMountFileInContainer(setting.pathInHost,
                                                  setting.pathInContainer,
                                                  setting.readOnly))) {
            log_error() << "Could not bind mount file into container";
            return false;
        }
    }
    return true;
}

bool FileGateway::teardownGateway()
{
    return true;
}
