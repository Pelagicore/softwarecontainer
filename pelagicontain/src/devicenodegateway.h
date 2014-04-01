/*
 *   Copyright (C) 2014 Pelagicore AB
 *   All rights reserved.
 */
#ifndef DEVICENODEGATEWAY_H
#define DEVICENODEGATEWAY_H
#include "jansson.h"

#include "gateway.h"
#include "controllerinterface.h"

/*! This gateway is responsible for exposing device nodes in an LXC container.
 * The basic operation looks as follows:
 * - DeviceNodeGateway (DNG) is loaded with a JSON configuration detailing a
 *   list of devices to create, complete with device names, permission modes,
 *   major and minor numbers. This is done via setConfig()
 * - DNG performs a rough verification of the configuration received, checking
 *   that all the required values are present. This is done in setConfig()
 * - Now the user may call activate(), which causes DNG to call
 *   ControllerAbstractInterface::systemCall() for:
 *      -# mknod of the configured devices
 *      -# chmod of the configured devices
 *
 * Notes:
 * - The entire list of devices supplied via setConfig() must be valid. If
 *   any entry is invalid, setConfig() will fail.
 * - activate() will stop issuing mknod and chmod upon first failure when
 *   iterating through the list created by setConfig
 *
 * JSON format for setConfig():
 * \code{.js}
 *  {"devices": [
 *                    {
 *                        "name":  "tty0",
 *                        "major": "4",
 *                        "minor": "0",
 *                        "mode":  "666"
 *                    },
 *                    {
 *                        "name":  "tty1",
 *                        "major": "4",
 *                        "minor": "0",
 *                        "mode":  "400"
 *                    },
 *                    {
 *                        "name":  "/dev/galcore",
 *                        "major": "199",
 *                        "minor": "0",
 *                        "mode":  "666"
 *                    }
 *                ]
 *  }
 * \endcode
 *
 * .. or in text:
 * - There is a root object called "devices".
 * - "devices" is a list of "device" objects
 * - Each "device" must contain:
 *  -# name: The name of the device. With or without path. This is passed
 *           verbatim to mknod
 *  -# major: The major device number, passed verbatim to mknod
 *  -# minor: The minor device number, passed verbatim to mknod
 *  -# mode: Permission mode, passed verbatim to chmod
 *
*/
class DeviceNodeGateway : public Gateway
{
public:
    DeviceNodeGateway(ControllerAbstractInterface *controllerInterface);
    ~DeviceNodeGateway(){}

    /*!
     *  Implements Gateway::id
     */
    virtual std::string id();

    /*!
     *  Implements Gateway::setConfig
     *
     *  Parse a JSON configuration string and create an internal list of
     *  devices from it. The JSON string is parsed and verified to be free of
     *  any obvious errors (missing keys, etc). If any device is malformed,
     *  parsing halts and false is returned. See class summary for an example
     *  JSON object.
     *
     * \param config JSON configuration object
     * \returns true upon successful parsing, false otherwise
     */
    virtual bool setConfig(const std::string &config);

    /*!
     *  Implements Gateway::activate
     *
     * This function will iterate over all devices configured using setConfig()
     * and issue mknod and chmod commands using
     * ControllerAbstractInterface::systemCall(), which are run in the
     * container. Calls to ControllerAbstractInterface::systemCall() are
     * executed in sequence, and if a call fails, any subsequent calls will not
     * be executed.
     *
     * \return true upon success; all commands executed successfully, false
     *              otherwise
     */
    virtual bool activate();

    /*! Implements Gateway::environment
     *
     * This gateway has no environment.
     */
    virtual std::string environment();

private:
    struct Device {
        std::string name;
        std::string major;
        std::string minor;
        std::string mode;
    };

    std::vector<struct DeviceNodeGateway::Device> m_devList;

    std::vector<struct Device> parseDeviceList(json_t *list, bool &ok);

    ControllerAbstractInterface  *m_controllerIface;
};

#endif /* DEVICENODEGATEWAY_H */
