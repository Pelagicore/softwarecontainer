/*
 *   Copyright (C) 2014 Pelagicore AB
 *   All rights reserved.
 */
#ifndef CONTROLLERABSTRACTINTERFACE_H
#define CONTROLLERABSTRACTINTERFACE_H

/*! ControllerAbstractInterface is an abstract interface to the Controller.
 *
 *  This class is used by Pelagicontain to communicate with Controller
 *  and is intended to be an abstraction of the acutal implementation.
 */
class ControllerAbstractInterface {

public:
	virtual ~ControllerAbstractInterface() {};

	/*! Starts the application inside the container
	 *
	 * \return True if all went well, false if not
	 */
	virtual bool startApp() = 0;

	/*! Stops the application running inside the container and also
	 *  stops Controller.
	 *
	 * \return True if all went well, false if not
	 */
	virtual bool shutdown() = 0;

	/*! Notifies the controller to issue the system call
	 *  defined in the cmd argument.
	 *
	 * \return True if all went well, false if not
	 */
	virtual bool systemCall(const std::string &cmd) const = 0;
};

#endif /* CONTROLLERABSTRACTINTERFACE_H */
