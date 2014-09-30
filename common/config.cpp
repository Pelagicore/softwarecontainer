/*
 *   Copyright (C) 2014 Pelagicore AB
 *   All rights reserved.
 */

#include "config.h"

Config*Config::m_instance = 0;

Config::Config() {
}

Config*Config::instance() {
    if (!m_instance) {
        m_instance = new Config();
    }

    return m_instance;
}
