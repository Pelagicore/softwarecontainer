/*
 *   Copyright (C) 2014 Pelagicore AB
 *   All rights reserved.
 */
#include <cstring>
#include <iostream>

#include "ipcmessage.h"

static const int OFFSET = 2;
static const int BUF_SIZE = 1024;
static const int PROTOCOL_INDEX = 0;
static const char RUN_APP = '1';
static const char KILL_APP = '2';
static const char SET_ENV_VAR = '3';
static const char SYS_CALL = '4';
static const char VAR_VAL_DELIMITER = ' ';

IPCMessage::IPCMessage(AbstractController &controller) :
    m_controller(controller)
{
}

IPCMessage::~IPCMessage()
{
}

bool IPCMessage::handleMessage(const char buf[], int length)
{
    char c;
    int total = 0;
    char msg[BUF_SIZE];

    // If there is one big message in the buffer is isn't allowed to be bigger
    // than the buffer we put it in.
    if (length > BUF_SIZE) {
        log_error() << "Total message buffer size too big";
        return false;
    }

    // Loop until the complete received buffer is processed, and find
    // (possibly many) null terminated messages in buffer
    bool done = false;
    while (!done) {
        memset(msg, '\0', sizeof(char) * BUF_SIZE);
        int i = 0;
        // Look for null terminated message in buffer
        do {
            c = buf[i + total];
            msg[i] = c;
            ++i;
        } while ((c != '\0') && ((i + total) < length));

        total += i;

        log_debug() << "Received \"" << msg << "\"";

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
    char variable[BUF_SIZE];
    memset(variable, 0, sizeof(char) * BUF_SIZE);

    char value[BUF_SIZE];
    memset(value, 0, sizeof(char) * BUF_SIZE);

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
    char command[BUF_SIZE];
    memset(command, 0, sizeof(char) * BUF_SIZE);

    strncpy(command, buf + OFFSET, messageLength);

    command[sizeof(command) - 1] = '\0';
    m_controller.systemCall(std::string(command));
}
