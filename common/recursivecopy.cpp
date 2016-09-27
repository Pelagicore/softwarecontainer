#include "recursivecopy.h"

#include <ftw.h>
#include <fstream>
#include <iostream>
#include <string>

std::string dst_root;
std::string src_root;

extern "C" int copy_file(const char*, const struct stat, int);

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
int copy_file(const char* src_path, const struct stat* sb, int typeflag) {
    std::string dst_path;
    std::string src(src_path);

    if (src.find(src_root) != std::string::npos) {
        dst_path = dst_root + src.erase(0, src_root.length());
    } else {
        dst_path = dst_root + src;
    }

    switch(typeflag) {
    case FTW_D:
        mkdir(dst_path.c_str(), sb->st_mode);
        break;
    case FTW_F:
        std::ifstream src(src_path, std::ios::binary);
        std::ofstream dst(dst_path, std::ios::binary);
        dst << src.rdbuf();
    }
    return 0;
}

RecursiveCopy &RecursiveCopy::getInstance() {
    static RecursiveCopy instance;
    return instance;
}

ReturnCode RecursiveCopy::copy(std::string src, std::string dst) {
    ReturnCode retval = ReturnCode::SUCCESS;
    m_copyLock.lock();
    dst_root.assign(dst);
    src_root.assign(src);
    if (ftw(src.c_str(), copy_file, 20) != 0)
    {
        log_error() << "Failed to recursively copy " << src << " to " << dst;
        retval = ReturnCode::FAILURE;
    }
    m_copyLock.unlock();

    return retval;
}

RecursiveCopy::RecursiveCopy() { }

RecursiveCopy::~RecursiveCopy() { }
