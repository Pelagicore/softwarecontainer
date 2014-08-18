/*
 *   Copyright (C) 2014 Pelagicore AB
 *   All rights reserved.
 */
#include <cstring>
#include <iostream>

#include "config.h"
#include "ipcmessage.h"
#include "pelagicontain-common.h"

static const int OFFSET = 2;
static const int PROTOCOL_INDEX = 0;


static const char VAR_VAL_DELIMITER = ' ';

IPCMessage::IPCMessage(AbstractController &controller) :
    m_controller(controller),
    m_bufferSize(Config::instance()->ipcBufferSize())
{
}

IPCMessage::~IPCMessage()
{
}

bool IPCMessage::handleMessage(const char buf[], int length)
{
    char c;
    int total = 0;
    char msg[m_bufferSize];

    // If there is one big message in the buffer is isn't allowed to be bigger
    // than the buffer we put it in.
    if (length > m_bufferSize) {
        log_error() << "Total message buffer size too big";
        return false;
    }

    // Loop until the complete received buffer is processed, and find
    // (possibly many) null terminated messages in buffer
    bool done = false;
    while (!done) {
        memset(msg, '\0', sizeof(char) * m_bufferSize);
        int i = 0;
        // Look for null terminated message in buffer
        do {
            c = buf[i + total];
            msg[i] = c;
            ++i;
        } while ((c != '\0') && ((i + total) < length));

        // Force null termination of message string
        if (msg[i] != '\0') {
            msg[i] = '\0';
        }
        log_debug() << "Received " << (const char*) msg;

        total += i;

        // If message is empty we don't need to handle it
        if (strlen(msg) > 0) {
            if (!dispatchMessage(msg)) {
                // The message was not understood by IPCMessage
                log_error() << "IPC message to Controller was not sent";
                return false;
            }
        }

        if (total == length) {
            done = true;
        }
    }

    // All messages were dispatched
    return true;
}

bool IPCMessage::dispatchMessage(const char buf[])
{
    bool retVal = false;

    switch (buf[PROTOCOL_INDEX]) {
    case RUN_APP:
        m_controller.runApp();
        retVal = true;
        break;
    case KILL_APP:
        m_controller.killApp();
        retVal = true;
        break;
    case SET_ENV_VAR:
        callSetEnvironmentVariable(buf, strlen(buf));
        retVal = true;
        break;
    case SYS_CALL:
        callSystemCall(buf, strlen(buf));
        retVal = true;
        break;
    default:
        retVal = false;
        break;
    }

    return retVal;
}

void IPCMessage::callSetEnvironmentVariable(const char buf[], int messageLength)
{
    char variable[m_bufferSize];
    memset(variable, 0, sizeof(char) * m_bufferSize);

    char value[m_bufferSize];
    memset(value, 0, sizeof(char) * m_bufferSize);

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

    m_controller.setEnvironmentVariable(variableString, valueString);
}

void IPCMessage::callSystemCall(const char buf[], int messageLength)
{
    char command[m_bufferSize];
    memset(command, 0, sizeof(char) * m_bufferSize);

    strncpy(command, buf + OFFSET, messageLength);

    command[sizeof(command) - 1] = '\0';
    m_controller.systemCall(std::string(command));
}
