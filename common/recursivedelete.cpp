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

#include "recursivedelete.h"

#include <ftw.h>
#include <fstream>
#include <iostream>
#include <string>

#include <glibmm.h>

namespace softwarecontainer {

std::string deleteRoot;

extern "C" int deleteFile(const char*, const struct stat, int);

/**
 * @brief deleteFile Delete files from the directory
 *
 * This function is wrapped and hidden in this separate code unit because of it's side effects
 * and used by the RecursiveDelete::delete() function.
 * @param deletePath The path of the file to delete
 * @param sb not used in this function
 * @param typeflag Points to what type of node is traversed by ftw().
 * @sideeffect Uses the global variables dst_root and src_root to know the root directories
 * being copied to and from. These must not be changed while ftw is running or the function will
 * misbehave.
 * @return always 0 for now.
 */
int deleteFile(const char* deletePath, const struct stat* sb, int typeflag)
{
    (void)sb;
    (void)typeflag;

    remove(deletePath);
    return 0;
}

RecursiveDelete &RecursiveDelete::getInstance()
{
    static RecursiveDelete instance;
    return instance;
}

bool RecursiveDelete::del(std::string dir)
{
    bool retval = true;
    m_deleteLock.lock();
    deleteRoot.assign(dir);
    if (ftw(dir.c_str(), deleteFile, 20) != 0) {
        log_error() << "Failed to recursively delete " << dir;
        retval = false;
    }
    m_deleteLock.unlock();

    return retval;
}

RecursiveDelete::RecursiveDelete() {}
RecursiveDelete::~RecursiveDelete() {}

} // namespace softwarecontainer
