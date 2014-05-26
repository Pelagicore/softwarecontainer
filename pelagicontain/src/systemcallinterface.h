/*
 *   Copyright (C) 2014 Pelagicore AB
 *   All rights reserved.
 */
#ifndef SYSTEMCALLINTERFACE_H
#define SYSTEMCALLINTERFACE_H

#include <string>
#include "systemcallabstractinterface.h"
#include "log.h"

/*! SystemcallInterface provides an interface to the host system.
 *
 *  This class is used by Pelagicontain to communicate with the host system
 *  and is intended to be an abstraction of the acutal implementation.
 */
class SystemcallInterface:
    public SystemcallAbstractInterface
{
	LOG_DECLARE_CLASS_CONTEXT("SYSC", "System call interface");

public:
    SystemcallInterface();
    ~SystemcallInterface();

    /*! Implements abstract method makeCall on SystemcallAbstractInterface
     *
     * \param cmd The command sent to system().
     * \return True if the command was successfully executed.
     */
    virtual bool makeCall(const std::string &cmd);

    /*! Implements abstract method makeCall on SystemcallAbstractInterface
     *
     * \param cmd The command sent to system().
     * \param exitCode Stores the exit code returned by the call to system()
     * \return True if the command was successfully executed.
     */
    virtual bool makeCall(const std::string &cmd, int &exitCode);

    /*! Implements abstract method makePopenCall on SystemcallAbstractInterface
     *
     * \param command The command send to popen().
     * \param infp A pointer to the input stream file descriptor stream.
     * \param infp A pointer to the output stream file descriptor stream.
     * \return pid_t if the command was successfully executed, otherwise -1.
     */
    virtual pid_t makePopenCall(const std::string &command,
                                int *infp,
                                int *outfp);

    /*! Implements abstract method makePcloseCall on SystemcallAbstractInterface
     *
     * \param pid A pid to the process to be closed.
     * \param infp The input stream file descriptor to be closed
     * \param infp The output stream file descriptor to be closed
     * \return True if the command was successfully executed.
     */
    virtual bool makePcloseCall(pid_t pid, int infp, int outfp);
};

#endif /* SYSTEMCALLINTERFACE_H */
