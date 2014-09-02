/*
 *   Copyright (C) 2014 Pelagicore AB
 *   All rights reserved.
 */
#include <cstring>
#include "ifaddrs.h"
#include "unistd.h"
#include "networkgateway.h"
#include "jansson.h"
#include "generators.h"


NetworkGateway::NetworkGateway(ControllerAbstractInterface
                               &controllerInterface,
                               SystemcallAbstractInterface
                               &systemCallInterface):
    Gateway(controllerInterface),
    m_ip(""),
    m_gateway(""),
    m_internetAccess(false),
    m_interfaceInitialized(false),
    m_systemCallInterface(systemCallInterface)
{
}

NetworkGateway::~NetworkGateway()
{
}

std::string NetworkGateway::id()
{
    return "network";
}

bool NetworkGateway::setConfig(const std::string &config)
{
    bool success = true;

    if (isInternetAccessSet(config)) {
        log_debug("Internet access will be enabled");
        m_internetAccess = true;
    } else {
        log_debug("Internet access disabled");
        m_internetAccess = false;
    }

    m_gateway = gatewayFromConfig(config);

    if (m_gateway.compare("") != 0) {
        log_debug("Default gateway set to %s", m_gateway.c_str());
    } else {
        m_internetAccess = false;
        log_debug("No gateway. Network access will be disabled");

        if (m_internetAccess) {
            log_error("Bad gateway setting in configuration file");
            success = false;
        }
    }

    return success;
}

bool NetworkGateway::activate()
{
    bool success = false;
    bool ready = false;

    if (isBridgeAvailable()) {
        if (generateIP()) {
            ready = true;
        }
    }

    if (ready) {
        if (m_internetAccess) {
            success = up();
        } else {
            success = down();
        }
    }

    return success;
}

const std::string NetworkGateway::ip()
{
    return m_ip;
}

bool NetworkGateway::generateIP()
{
    const char * ipAddrNet = m_gateway.substr(0, m_gateway.size() - 1).c_str();

    m_ip = Generator::gen_ip_addr(ipAddrNet);
    log_debug("IP set to %s", m_ip.c_str());

    return true;
}

bool NetworkGateway::setDefaultGateway()
{
    std::string cmd = "route add default gw ";
    getController().systemCall(cmd + m_gateway);

    return true;
}

bool NetworkGateway::up()
{
    std::string cmd;

    if (!m_interfaceInitialized) {
        cmd = "ifconfig eth0 " + m_ip + " netmask 255.255.255.0 up";
        m_interfaceInitialized = true;
    } else {
        cmd = "ifconfig eth0 up";
    }

    getController().systemCall(cmd);

    /* The route to the default gateway must be added
       each time the interface is brought up again */
    return setDefaultGateway();
}

bool NetworkGateway::down()
{
    std::string cmd = "ifconfig eth0 down";
    getController().systemCall(cmd);

    return true;
}

bool NetworkGateway::isBridgeAvailable()
{
    bool ret = false;
    std::string cmd = "ifconfig | grep -C 2 \"container-br0\" | grep -q \"" + m_gateway + "\"";

    if (m_systemCallInterface.makeCall(cmd)) {
        ret = true;
    } else {
        log_error("No network bridge configured");
    }

    return ret;
}

/* NOTE: This code has not been tested since it was moved here from it's
 * original place.
 */
int NetworkGateway::waitForDevice(const std::string &iface)
{
    struct ifaddrs *ifaddr, *ifa;
    int max_poll = 10;
    bool found_iface = false;
    int i = 0;
    int retval = 0;

    while (i < max_poll && found_iface == false) {
        if (getifaddrs(&ifaddr) == -1) {
            retval = -EINVAL;
        	log_error("getifaddrs") << strerror(errno);
            goto cleanup_wait;
        }

        /* Iterate through the device list */
        for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
            if (ifa->ifa_name == NULL) {
                continue;
            } else if (strcmp(ifa->ifa_name, iface.c_str()) == 0) {
                log_debug("Device found: %s", ifa->ifa_name);
                found_iface = true;
                break;
            }
        }

        if (!found_iface) {
            log_debug("Device unavailable");
        }

        /* Give the device some time to show up */
        usleep(250000);
        i++;
    }

    retval = found_iface;
cleanup_wait:
    return retval;
}

bool NetworkGateway::isInternetAccessSet(const std::string &config)
{
    json_error_t error;
    json_t *root, *value;
    bool accessSet = false;

    // Get root JSON object
    root = json_loads(config.c_str(), 0, &error);

    if (!root) {
        log_error("Error on line %d: %s", error.line, error.text);
        goto cleanup_parse_json;
    }

    // Get value
    value = json_object_get(root, "internet-access");
    if (!json_is_boolean(value)) {
        log_error("Value is not a boolean.");
        json_decref(value);
        goto cleanup_parse_json;
    }

    accessSet = json_is_true(value);

cleanup_parse_json:
    if (root) {
        json_decref(root);
    }

    return accessSet;
}

std::string NetworkGateway::gatewayFromConfig(const std::string &config)
{
    json_error_t error;
    json_t *root, *value;
    std::string gateway = "";

    // Get root JSON object
    root = json_loads(config.c_str(), 0, &error);

    if (!root) {
        log_error("Error on line %d: %s", error.line, error.text);
        goto cleanup_parse_json;
    }

    // Get value
    value = json_object_get(root, "gateway");

    if (!json_is_string(value)) {
        log_error("Value is not a string.");
        json_decref(value);
        goto cleanup_parse_json;
    }

    gateway = json_string_value(value);

cleanup_parse_json:
    if (root) {
        json_decref(root);
    }

    return gateway;
}
