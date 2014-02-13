/*
 *   Copyright (C) 2014 Pelagicore AB
 *   All rights reserved.
 */
#ifndef MAINLOOPABSTRACTINTERFACE_H
#define MAINLOOPABSTRACTINTERFACE_H

class MainloopAbstractInterface {

public:
	virtual ~MainloopAbstractInterface() {};

	virtual void enter() = 0;
	virtual void leave() = 0;
};

#endif /* MAINLOOPABSTRACTINTERFACE_H */
