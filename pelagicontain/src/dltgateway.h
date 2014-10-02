/*
 *   Copyright (C) 2014 Pelagicore AB
 *   All rights reserved.
 */
#pragma once

#include <string>
#include "gateway.h"

/**
 * To be removed
 */
class DLTGateway : public Gateway {

    LOG_DECLARE_CLASS_CONTEXT("DLTG", "DLT gateway");

public:
    static constexpr const char *ID = "dlt";
    static constexpr const char *DLT_SOCKET_FOLDER = "/tmp/";
    static constexpr const char *SOCKET_FILE_NAME = "dlt";

    DLTGateway() {
    }

    ~DLTGateway() {
    }

    std::string id() override {
        return ID;
    }

    bool setConfig(const std::string &config) override {
        JSonElement parser(config);

        parser.readBoolean(ENABLED_FIELD, m_enabled);

        log_debug() << "Received config enabled: " << m_enabled;

        return true;
    }

    bool activate() override {
        if (m_enabled) {
            log_error() << "enabling DLT gateway";
            std::string d = logging::StringBuilder() << DLT_SOCKET_FOLDER << "/" << SOCKET_FILE_NAME;
            std::string path = getContainer().bindMountFileInContainer(d, SOCKET_FILE_NAME, false);
            createSymLinkInContainer(path, d);
        }

        return true;
    }

    bool teardown() override {
        return true;
    }

private:
    bool m_enabled = false;
};
