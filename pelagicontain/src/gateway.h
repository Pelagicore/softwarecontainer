/*
 *   Copyright (C) 2014 Pelagicore AB
 *   All rights reserved.
 */
#ifndef GATEWAY_H
#define GATEWAY_H

#include <string>
#include "controllerinterface.h"
#include "container.h"

#include <wait.h>

/*! Gateway base class
 *
 * Gateway base class for Pelagicontain
 */
class Gateway
{
	LOG_DECLARE_CLASS_CONTEXT("GATE", "Gateway");

protected:
	static constexpr const char* ENABLED_FIELD = "enabled";

public:
    Gateway(ControllerAbstractInterface &controllerInterface):
        m_controllerInterface(controllerInterface){
    };

    virtual ~Gateway() {
    };

    /*! Used by pelagicontain to map configurations to gateways */
    virtual std::string id() = 0;

    /*! Configure this gateway according to the supplied JSON configuration
     * string
     *
     * \param config JSON string containing gatway-specific JSON configuration
     * \returns true if \p config was sucecssfully parsed
     *          false otherwise
     */
    virtual bool setConfig(const std::string &config) = 0;

    /*! Applies any configuration set by setConfig()
     *
     * \returns true upon successful application of configuration
     *          false otherwise
     */
    virtual bool activate() = 0;

    /*! Restore system to the state prior to launching of gateway. Any cleanup
     * code (removal of files, virtual interfaces, etc) should be placed here.
     *
     * \returns true upon successful clean-up, false otherwise
     */
    virtual bool teardown() {
        return true;
    }

	ReturnCode createSymLinkInContainer(const std::string &source, const std::string &destination ) {

		auto m_pid = getContainer().executeInContainer([=]() {
			log_debug() << "symlink " << source << " to " << destination;
			auto r = symlink(source.c_str() , destination.c_str());
			if (r != 0)
				log_error() << "Can't create symlink " << source << " to " << destination;
			return r;
		});

		int status;
		waitpid(m_pid, &status, 0);
		return (status == 0 ) ? ReturnCode::SUCCESS : ReturnCode::FAILURE;
	}

    ControllerAbstractInterface& getController() {
    	return m_controllerInterface;
    }

    Container& getContainer() {
    	return *m_container;
    }

    void setContainer(Container& container) {
    	m_container = &container;
    }

protected:
    ControllerAbstractInterface &m_controllerInterface;
    Container* m_container = nullptr;
};

#endif /* GATEWAY_H */
