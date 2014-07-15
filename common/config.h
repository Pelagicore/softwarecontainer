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
    int m_ipcBufferSize;
    int m_controllerConnectionTimeout;
};

#endif // CONFIG_H
