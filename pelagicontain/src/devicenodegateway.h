/*
 *   Copyright (C) 2014 Pelagicore AB
 *   All rights reserved.
 */
#ifndef DEVICENODEGATEWAY_H
#define DEVICENODEGATEWAY_H
#include "jansson.h"

#include "gateway.h"

/*! This gateway is responsible for exposing device nodes in an LXC container.
 * The basic operation looks as follows:
 * - DeviceNodeGateway (DNG) is loaded with a JSON configuration detailing a
 *   list of devices to create, with device names (mandatory), permission modes (optional),
 *   major  (optional) and minor numbers  (optional).
 *
 * Notes:
 * - activate() will stop creating devices in the container upon first failure
 *
 * JSON format :
 * \code{.js}
 *  {[
 *                    {
 *                        "name":  "/dev/dri/card0"
 *                    },
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
class DeviceNodeGateway :
    public Gateway
{
    LOG_DECLARE_CLASS_CONTEXT("DNG", "Device node gateway");

public:
    DeviceNodeGateway();
    ~DeviceNodeGateway()
    {
    }

    /*!
     *  Implements Gateway::id
     */
    virtual std::string id()
    {
        return "devicenode";
    }

    ReturnCode readConfigElement(const JSonElement &element) override;

    /*!
     *  Implements Gateway::activate
     *
     * This function will iterate over all devices and issue mknod and chmod commands using
     * ControllerAbstractInterface::systemCall(), which are run in the
     * container. Calls to ControllerAbstractInterface::systemCall() are
     * executed in sequence, and if a call fails, any subsequent calls will not
     * be executed.
     *
     * \return true upon success; all commands executed successfully, false
     *              otherwise
     */
    virtual bool activate();

private:
    struct Device
    {
        std::string name;
        std::string major;
        std::string minor;
        std::string mode;
    };

    std::vector<DeviceNodeGateway::Device> m_devList;

};

#endif /* DEVICENODEGATEWAY_H */
