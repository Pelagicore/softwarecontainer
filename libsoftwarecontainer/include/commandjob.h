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
#include "jobabstract.h"
namespace softwarecontainer {

/**
 * Use this class to execute a command line in a SoftwareContainer.
 */
class CommandJob :
    public JobAbstract
{

public:
    static constexpr int UNASSIGNED_STREAM = -1;

    CommandJob(SoftwareContainerLib &lib, const std::string &command);
    virtual ~CommandJob();

    ReturnCode setWorkingDirectory(const std::string &folder);
    ReturnCode setUserID(uid_t userID);
    ReturnCode start();
    std::string toString() const;

private:
    std::string m_command;
    std::string m_workingDirectory;
    uid_t m_userID = ROOT_UID;
};
}
