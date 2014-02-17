/*
 *   Copyright (C) 2014 Pelagicore AB
 *   All rights reserved.
 */
#ifndef MAINLOOPABSTRACTINTERFACE_H
#define MAINLOOPABSTRACTINTERFACE_H

/*! Abstraction of the mainloop which is used to hide details from Pelagicontain
 *
 * Pelagicontain needs to e.g. exit the mainloop on during shutdown but should
 * not know about implementation details.
 */
class MainloopAbstractInterface {

public:
	virtual ~MainloopAbstractInterface() {};

	virtual void enter() = 0;
	virtual void leave() = 0;
};

#endif /* MAINLOOPABSTRACTINTERFACE_H */
