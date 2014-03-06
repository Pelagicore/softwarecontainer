/*
 *   Copyright (C) 2014 Pelagicore AB
 *   All rights reserved.
 */
class Controller {

public:
    Controller();
    ~Controller();

    bool initialize(const std::string &fifoPath);

private:
    bool loop();
    bool createFifo();
    int runApp();
    void killApp();

    pid_t m_pid;
    std::string m_fifoPath;
    int m_fifo;
};
