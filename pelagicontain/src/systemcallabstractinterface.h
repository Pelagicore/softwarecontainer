/*
 *   Copyright (C) 2014 Pelagicore AB
 *   All rights reserved.
 */
#ifndef SYSTEMCALLABSTRACTINTERFACE_H
#define SYSTEMCALLABSTRACTINTERFACE_H

#include <string>

/*! SystemcallAbstractInterface provides an abstract interface to the system.
 *
 *  This class is used by Pelagicontain to communicate with the host system
 *  and is intended to be an abstraction of the acutal implementation.
 */
class SystemcallAbstractInterface {

public:

    virtual ~SystemcallAbstractInterface() {
    };

    /*! Issues a call to system() with the command passed as argument
     *
     * \param cmd The command sent to system().
     * \return True if the command was successfully executed.
     */
    virtual bool makeCall(const std::string &cmd) = 0;

    /*! Issues a call to system() with the command passed as argument
     *
     * \param cmd The command sent to system().
     * \param exitCode Stores the exit code returned by the call to system()
     * \return True if the command was successfully executed.
     */
    virtual bool makeCall(const std::string &cmd, int &exitCode) = 0;

    /*! Issues a call to makePopenCall() with the command passed as argument
     *
     * \param command The command send to popen().
     * \param infp A pointer to the input stream file descriptor stream.
     * \param infp A pointer to the output stream file descriptor stream.
     * \return pid_t if the command was successfully executed, otherwise -1.
     */
    virtual pid_t makePopenCall(const std::string &command,
                                int *infp,
                                int *outfp) = 0;

    /*! Issues a call to makePcloseCall() with the file descriptor as a argument
     *
     * \param pid A pid to the process to be closed.
     * \param infp The input stream file descriptor to be closed
     * \param infp The output stream file descriptor to be closed
     * \return True if the command was successfully executed.
     */
    virtual bool makePcloseCall(pid_t pid, int infp, int outfp) = 0;


};

#endif /* SYSTEMCALLABSTRACTINTERFACE_H */
