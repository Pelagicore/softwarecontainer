/*
 *   Copyright (C) 2014 Pelagicore AB
 *   All rights reserved.
 */
#include <cstring>
#include <iostream>

#include "ipcmessage.h"

static const int OFFSET = 2;
static const int ERROR = -1;
static const int SUCCESS = 0;
static const int BUF_SIZE = 1024;
static const int PROTOCOL_INDEX = 0;
static const char RUN_APP = '1';
static const char KILL_APP = '2';
static const char SET_ENV_VAR = '3';
static const char SYS_CALL = '4';
static const char VAR_VAL_DELIMITER = ' ';

IPCMessage::IPCMessage(AbstractController *controller) :
    m_controller(controller)
{
}

IPCMessage::~IPCMessage()
{
}

bool IPCMessage::handleMessage(const std::string &message, int *statusFlag)
{
    const char *buf = message.c_str();
    bool retVal = false;

    // Flag is set to ERROR only when the message was not understood
    *statusFlag = SUCCESS;

    switch (buf[PROTOCOL_INDEX]) {
    case RUN_APP:
        m_controller->runApp();
        retVal = true;
        break;
    case KILL_APP:
        m_controller->killApp();
        // When app is shut down, we return 'false' to indicate controller
        // should go on shutting down and the IPC should not pass any more
        // messages.
        retVal = false;
        break;
    case SET_ENV_VAR:
        callSetEnvironmentVariable(buf, message.size());
        retVal = true;
        break;
    case SYS_CALL:
        callSystemCall(buf, message.size());
        retVal = true;
        break;
    default:
        // The message had no meaning to us, this is an error but the IPC
        // should continue to pass messages.
        *statusFlag = ERROR;
        retVal = true;
        break;
    }

    return retVal;
}

void IPCMessage::callSetEnvironmentVariable(const char *buf, int messageLength)
{
    char variable[BUF_SIZE];
    memset(variable, 0, sizeof(variable));

    char value[BUF_SIZE];
    memset(value, 0, sizeof(value));

    // Find the variable and the value
    for (int i = OFFSET; i < messageLength; ++i) {
        if (buf[i] == VAR_VAL_DELIMITER) {
            // We're between the variable and the value
            int separator = i;
            strncpy(variable, buf + OFFSET, separator - OFFSET);
            strncpy(value, buf + OFFSET + separator - 1, messageLength);
            break;
        }
    }

    variable[sizeof(variable) - 1] = '\0';
    std::string variableString(variable);

    value[sizeof(value) - 1] = '\0';
    std::string valueString(value);

    m_controller->setEnvironmentVariable(variableString, valueString);
}

void IPCMessage::callSystemCall(const char *buf, int messageLength)
{
        char command[BUF_SIZE];
        memset(command, 0, sizeof(command));

        strncpy(command, buf + OFFSET, messageLength);

        command[sizeof(command) - 1] = '\0';
        m_controller->systemCall(std::string(command));
}
