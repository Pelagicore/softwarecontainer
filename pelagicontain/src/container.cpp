/*
 *   Copyright (C) 2014 Pelagicore AB
 *   All rights reserved.
 */
#include <fstream>
#include <unistd.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "container.h"

#ifndef LXCTEMPLATE
    #error Must define LXCTEMPLATE as path to lxc-pelagicontain
#endif

/* A directory need to exist and be set up in a special way.
 *
 * Basically something like:
 * mkdir -p <containerRoot>/late_mounts
 * mount --bind <containerRoot>/late_mounts <containerRoot>/late_mounts
 * mount --make-unbindable <containerRoot>/late_mounts
 * mount --make-shared <containerRoot>/late_mounts
 */

Container::Container(const std::string &name,
                     const std::string &configFile,
                     const std::string &containerRoot):
    m_configFile(configFile),
    m_name(name),
    m_containerRoot(containerRoot),
    m_mountDir(containerRoot + "/late_mounts")
{
}

bool Container::initialize()
{
    // Make sure the directory for "late mounts" exist
    if (!isDirectory(m_mountDir.c_str())) {
        log_error() << "Directory " << m_mountDir << " does not exist";
        return false;
    }

    // Create directories needed for mounting, will be removed in dtor
    std::string runDir = m_mountDir + "/" + m_name;

    bool allOk = true;
    allOk = allOk && createDirectory(runDir.c_str());
    allOk = allOk && createDirectory((runDir + "/bin").c_str());
    allOk = allOk && createDirectory((runDir + "/shared").c_str());
    allOk = allOk && createDirectory((runDir + "/home").c_str());

    if (!allOk) {
        log_error() << "Could not set up all needed directories";
        return false;
    }

    return true;
}

bool Container::createDirectory(const std::string &path)
{
    if (mkdir(path.c_str(), S_IRWXU) == -1) {
        log_error("Could not create directory %s, %s.", path.c_str(), strerror(errno));
        return false;
    }

    m_dirs.push_back(path);
    return true;
}

bool Container::isDirectory(const std::string &path)
{
    bool isDir = false;
    struct stat st;
    if (stat(path.c_str(), &st) == 0) {
        if ((st.st_mode & S_IFDIR) != 0) {
            isDir = true;
        }
    }
    return isDir;
}

Container::~Container()
{

    // Unmount all mounted dirs. Done backwards, behaving like a stack.
    for (std::vector<std::string>::const_reverse_iterator it = m_mounts.rbegin();
         it != m_mounts.rend();
         ++it)
    {
        log_debug() << "Unmounting " << (*it).c_str();
        if (umount((*it).c_str()) == -1) {
            log_error("Could not unmount %s, %s", (*it).c_str(), strerror(errno));
        }
    }

    // Clean up all created directories, also done backwards
    for (std::vector<std::string>::const_reverse_iterator it = m_dirs.rbegin();
         it != m_dirs.rend();
         ++it)
    {
        log_debug() << "Removing " << (*it).c_str();
        if (rmdir((*it).c_str()) == -1) {
            log_error("Could not remove dir %s, %s", (*it).c_str(), strerror(errno));
        }
    }
}

const char *Container::name()
{
    return m_name.c_str();
}

std::vector<std::string> Container::commands(const std::string &containedCommand)
{
    int max_cmd_len = sysconf(_SC_ARG_MAX);
    char lxc_command[max_cmd_len];
    std::vector<std::string> commands;

    // Command to create container
    sprintf(lxc_command,
            "CONTROLLER_DIR=%s GATEWAY_DIR=%s MOUNT_DIR=%s lxc-create -n %s -t %s"
            " -f %s > /tmp/lxc_%s.log",
            (m_containerRoot + "/bin").c_str(),
            (m_containerRoot + name() + "/gateways/").c_str(),
            m_mountDir.c_str(),
            name(),
            LXCTEMPLATE,
            m_configFile.c_str(),
            name());
    commands.push_back(std::string(lxc_command));

    // Create command to execute inside container
    snprintf(lxc_command, max_cmd_len,
             "lxc-execute -n %s -- env %s",
             name(),
             containedCommand.c_str());
    commands.push_back(std::string(lxc_command));

    // Command to destroy container
    snprintf(lxc_command, max_cmd_len, "lxc-destroy -f -n %s", name());
    commands.push_back(std::string(lxc_command));

    return commands;
}

bool Container::bindMountDir(const std::string &src, const std::string &dst)
{
    int mountRes = mount(src.c_str(), // source
                         dst.c_str(), // target
                         "",          // fstype
                         MS_BIND,     // flags
                         NULL);       // data

    if (mountRes == 0) {
        // Success
        m_mounts.push_back(dst);
    } else {
        // Failure
        log_error("Could not mount dir into container: src=%s, dst=%s err=%s",
                  src.c_str(),
                  dst.c_str(),
                  strerror(errno));
    }

    return (mountRes == 0);
}

/* When we know which app that will be run we need to
 * do some setup, like mount in the application bin and
 * shared directories.
 */
bool Container::setApplication(const std::string &appId)
{
    // The directory(ies) to be mounted is known by convention, e.g.
    // /var/am/<appId>/bin/ and /var/am/<appId>/shared/

    // bind mount /var/am/<appId>/bin/ into /var/am/late_mounts/<contid>/bin
    // this directory will be accessible in container as according to
    // the lxc-pelagicontain template
    std::string appDirBase = m_containerRoot + "/" + appId;
    std::string dstDirBase = m_mountDir + "/" + m_name;

    // If any bind mout call fails there's no need to make any remaining calls
    bool allOk = true;
    allOk = allOk && bindMountDir(appDirBase + "/bin", dstDirBase + "/bin");
    allOk = allOk && bindMountDir(appDirBase + "/shared", dstDirBase + "/shared");
    allOk = allOk && bindMountDir(appDirBase + "/home", dstDirBase + "/home");

    return allOk;
}
