/*
 *   Copyright (C) 2014 Pelagicore AB
 *   All rights reserved.
 */
#ifndef CONTROLLERINTERFACE_H
#define CONTROLLERINTERFACE_H

/*! ControllerInterface is an interface to Controller.
 *
 *  This class is used by Pelagicontain to communicate with Controller
 *  and is intended to hide the details of the communication mechanism
 *  implementation.
 */
class ControllerInterface
{
public:
	/*! Starts the application inside the container
	 *
	 * \return True if all went well, false if not
	 */
	static bool startApp();

	/*! Stops the application running inside the container and also
	 *  stops Controller.
	 *
	 * \return True if all went well, false if not
	 */
	static bool shutdown();
};

#endif /* CONTROLLERINTERFACE_H */
