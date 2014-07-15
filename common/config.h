/*
 *   Copyright (C) 2014 Pelagicore AB
 *   All rights reserved.
 */

#ifndef CONFIG_H
#define CONFIG_H

class Config {
public:
    static Config *instance();
    int ipcBufferSize();
    int controllerConnectionTimeout();

private:
    Config();
    Config(Config const &);
    Config &operator=(Config const &);

    static Config *m_instance;

    // Buffer size used in IPC code.
    int m_ipcBufferSize;

    // Timeout in seconds for how long Pelagicontain will wait for Controller
    // to connect.
    int m_controllerConnectionTimeout;
};

#endif // CONFIG_H
