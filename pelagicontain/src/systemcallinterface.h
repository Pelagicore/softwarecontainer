/*
 *   Copyright (C) 2014 Pelagicore AB
 *   All rights reserved.
 */
#ifndef SYSTEMCALLINTERFACE_H
#define SYSTEMCALLINTERFACE_H

#include <string>
#include "systemcallabstractinterface.h"

/*! SystemcallInterface provides an interface to the host system.
 *
 *  This class is used by Pelagicontain to communicate with the host system
 *  and is intended to be an abstraction of the acutal implementation.
 */
class SystemcallInterface:
    public SystemcallAbstractInterface
{
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

    /*! Implements abstract method popen on SystemcallAbstractInterface
	 *
     * \param command The command send to popen().
	 * \param type The type send to popen().
     * \param fd A pointer to the file descriptor stream.
	 * \return True if the command was successfully executed.
     */
    virtual bool makePopenCall(const std::string &command, const std::string &type, FILE **fd);

    /*! Implements abstract method pclose on SystemcallAbstractInterface
     *
     * \param fd A pointer to the file descriptor stream.
     * \param exitCode Stores the exit code returned by the call to pclose()
     * \return True if the command was successfully executed.
     */
    virtual bool makePcloseCall(FILE **fd, int &exitCode);
};

#endif /* SYSTEMCALLINTERFACE_H */
