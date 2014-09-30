/*
 *   Copyright (C) 2014 Pelagicore AB
 *   All rights reserved.
 */
#include "gateway.h"

ReturnCode Gateway::createSymLinkInContainer(const std::string &source, const std::string &destination) {

    auto m_pid = getContainer().executeInContainer([ = ]() {
                                                       log_debug() << "symlink " << source << " to " << destination;
                                                       auto r = symlink( source.c_str(), destination.c_str() );
                                                       if (r != 0)
                                                           log_error() << "Can't create symlink " << source << " to " <<
                                                           destination;
                                                       return r;
                                                   });

    int status;
    waitpid(m_pid, &status, 0);
    return (status == 0) ? ReturnCode::SUCCESS : ReturnCode::FAILURE;
}
