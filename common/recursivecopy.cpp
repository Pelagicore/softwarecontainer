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

#include "recursivecopy.h"

#include <ftw.h>
#include <fstream>
#include <iostream>
#include <string>

namespace softwarecontainer {

std::string dstRoot;
std::string srcRoot;

extern "C" int copyFile(const char*, const struct stat, int);

/**
 * @brief copy_file Copy files from src_path to dst_root global path
 *
 * This function is wrapped and hidden in this separate code unit because of it's side effects
 * and used by the RecursiveCopy::copy() function.
 * @param src_path The path of the file to copy
 * @param sb not used in this function
 * @param typeflag Points to what type of node is traversed by ftw().
 * @sideeffect Uses the global variables dst_root and src_root to know the root directories
 * being copied to and from. These must not be changed while ftw is running or the function will
 * misbehave.
 * @return always 0 for now.
 */
int copyFile(const char* srcPath, const struct stat* sb, int typeflag)
{
    std::string dstPath;
    std::string src(srcPath);

    if (src.find(srcRoot) != std::string::npos) {
        dstPath = dstRoot + src.erase(0, srcRoot.length());
    } else {
        dstPath = dstRoot + src;
    }

    switch(typeflag) {
    case FTW_D:
        mkdir(dstPath.c_str(), sb->st_mode);
        break;
    case FTW_F:
        std::ifstream src(srcPath, std::ios::binary);
        std::ofstream dst(dstPath, std::ios::binary);
        dst << src.rdbuf();
    }
    return 0;
}

RecursiveCopy &RecursiveCopy::getInstance()
{
    static RecursiveCopy instance;
    return instance;
}

ReturnCode RecursiveCopy::copy(std::string src, std::string dst)
{
    ReturnCode retval = ReturnCode::SUCCESS;
    m_copyLock.lock();
    dstRoot.assign(dst);
    srcRoot.assign(src);
    if (ftw(src.c_str(), copyFile, 20) != 0) {
        log_error() << "Failed to recursively copy " << src << " to " << dst;
        retval = ReturnCode::FAILURE;
    }
    m_copyLock.unlock();

    return retval;
}

RecursiveCopy::RecursiveCopy() { }

RecursiveCopy::~RecursiveCopy() { }

} // namespace softwarecontainer
