/*
 *   Copyright (C) 2014 Pelagicore AB
 *   All rights reserved.
 */
#pragma once

#include <string>
#include <unistd.h>
#include "gateway.h"
#include "systemcallinterface.h"

class FileGateway : public Gateway {

    LOG_DECLARE_CLASS_CONTEXT("File", "file gateway");

public:
    static constexpr const char *ID = "file";

    FileGateway() {
    }

    ~FileGateway() {
    }

    std::string id() override {
        return ID;
    }

    bool setConfig(const std::string &config) override;

    bool activate() override {
        return true;
    }

    bool teardown() override {
        return true;
    }

private:
    bool m_enabled = false;
};
