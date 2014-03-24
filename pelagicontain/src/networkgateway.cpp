/*
 *   Copyright (C) 2014 Pelagicore AB
 *   All rights reserved.
 */
#include <cstring>
#include "ifaddrs.h"
#include "unistd.h"
#include "debug.h"
#include "networkgateway.h"
#include "jansson.h"
#include "generators.h"


NetworkGateway::NetworkGateway(ControllerAbstractInterface *controllerInterface):
    Gateway(controllerInterface),
    m_ip(""),
    m_gateway(""),
    m_internetAccess(false),
    m_interfaceInitialized(false)
{
}

NetworkGateway::~NetworkGateway()
{
}

std::string NetworkGateway::id()
{
    return "networking";
}

std::string NetworkGateway::environment()
{
    return "";
}

bool NetworkGateway::setConfig(const std::string &config)
{
    log_debug("NetworkGateway::setConfig called\n");

    bool success = true;
    
    /* Check the value of the internet-access key of the config*/
    std::string value = parseConfig(config.c_str(), "internet-access");
    if (value.compare("true") == 0)
    {
	log_debug("Internet access will be enabled\n");
	m_internetAccess = true;
    }
    else
    {
	log_debug("Internet access disabled\n");
	m_internetAccess = false;
    }

    /* Retrieve the gateway IP */
    m_gateway = parseConfig(config.c_str(), "gateway");

    if (m_gateway.compare("") != 0)
    {
	success = isBridgeAvailable();
	log_debug("Default gateway set to %s\n", m_gateway.c_str());

	if (!setContainerIP())
	{
	    success = false;
	}
    }
    else
    {
	log_debug("No gateway setting in configuration file\n");
	log_debug("Network access will be disabled\n");
	m_internetAccess = false;
    }

    return success;
}

bool NetworkGateway::activate()
{
    bool success = false;

    if (m_internetAccess)
    {
	success = up();
    }

    else
    {
	success = down();
    }
    
    return success;
}

std::string NetworkGateway::ip()
{
    return m_ip;
}

bool NetworkGateway::setContainerIP()
{
    const char * ipAddrNet = m_gateway.substr(0, m_gateway.size() - 1).c_str();
    
    m_ip = gen_ip_addr(ipAddrNet);
    log_debug("IP set to %s\n", m_ip.c_str());

    return true;
}

bool NetworkGateway::setDefaultGateway()
{
    std::string cmd = "route add default gw ";
    m_controllerInterface->systemCall(cmd + m_gateway);

    return true;
}

bool NetworkGateway::up()
{
    std::string cmd;

    if (!m_interfaceInitialized)
    {
	cmd = "ifconfig eth0 " + m_ip + " netmask 255.255.255.0 up";
	m_interfaceInitialized = true;
    }
    else
    {
	cmd = "ifconfig eth0 up";
    }
    
    m_controllerInterface->systemCall(cmd);

    /* The route to the default gateway must be added
       each time the interface is brought up again */
    return setDefaultGateway();
}

bool NetworkGateway::down()
{
    std::string cmd = "ifconfig eth0 down";
    m_controllerInterface->systemCall(cmd);

    return true;
}

bool NetworkGateway::ping(const std::string &ip)
{
    std::string cmd = "ping -c 2 ";
    m_controllerInterface->systemCall(cmd + ip);

    return true;
}

bool NetworkGateway::isBridgeAvailable()
{
    bool ret = false;
    std::string cmd = "ifconfig | grep -C 2 \"container-br0\" | grep -q \"" + m_gateway + "\"";

    if (system(cmd.c_str()) == 0)
    {
	ret = true;
    }
    else
    {
	log_error("No network bridge configured");
    }

    return ret;
}


/*
 * This is a wrapper around the tc command. This function will issue system ()
 * calls to tc.
 */
int NetworkGateway::limitIface(const std::string &ifaceName, const std::string &tcRate)
{
    pid_t pid = 0;

    /* fork */
    pid = fork();
    if (pid == 0) { /* child */
        char cmd[256];

        snprintf(cmd, sizeof(cmd), "tc qdisc "
                 "add "
                 "dev "
                 "%s "
                 "root "
                 "tbf "
                 "rate %s "
                 "burst 5kb "
                 "latency 70ms ",
                 ifaceName.c_str(),
                 tcRate.c_str());

        /* poll for device */
        if (!waitForDevice(ifaceName)) {
            log_error("Device never showed up. Not setting TC.\n");
            /* We're forked, so just exit */
            exit(0);
        }

        /* issue command */
        log_debug("issuing: %s\n", cmd);
        system(cmd);
        exit(0);

    } else { /* parent */
        if (pid == -1) {
            log_error("Unable to fork interface observer!\n");
            return -EINVAL;
        }
        return 0;
    }
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
            perror("getifaddrs");
            goto cleanup_wait;
        }

        /* Iterate through the device list */
        for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
            if (ifa->ifa_name == NULL) {
                continue;
            } else {
                if (strcmp(ifa->ifa_name, iface.c_str()) == 0) {
                    log_debug("Device found: %s\n", ifa->ifa_name);
                    found_iface = true;
                    break;
                }
            }
        }

        if (!found_iface)
            log_debug("Device unavailable");

        /* Give the device some time to show up */
        usleep(250000);
        i++;
    }

    retval = found_iface;
cleanup_wait:
    return retval;
}

/*
 * This function issues "tc qdisc del dev <device> root"
 */
int NetworkGateway::clearIfaceLimits(char *iface)
{
    int retval = 0;
    char cmd[256];

    snprintf(cmd, sizeof(cmd), "tc qdisc "
             "del "
             "dev "
             "%s "
             "root ",
             iface);

    if (system(cmd) == -1) {
        log_error("Unable to execute limit clear command\n");
        return -EINVAL;
    }

    return retval;
}

bool NetworkGateway::setupIptables(const std::string &ipAddress, const std::string &rules)
{
    char iptables_cmd[1024];
    char iptables_rules_file[] = "/tmp/iptables_rules_XXXXXX";
    int iptf = 0;

    iptf = mkstemp(iptables_rules_file);
    if (iptf == -1) {
        log_error("Unable to open %s", iptables_rules_file);
        return false;
    }

    if (write(iptf, rules.c_str(), sizeof(char) * strlen(rules.c_str())) == -1) {
        log_error("Failed to write rules file");
        goto unlink_file;
    }

    if (close(iptf) == 1) {
        log_error("Failed to close rules file");
        goto unlink_file;
    }

    /* Execute shell script with env variable set to container IP */
    snprintf(iptables_cmd, sizeof(iptables_cmd), "env SRC_IP=%s sh %s",
             ipAddress.c_str(), iptables_rules_file);

    log_error("Generating rules for IP: %s", ipAddress.c_str());
    if (system(iptables_cmd) == -1)
        log_error("Failed to execute iptables command");

unlink_file:
    unlink(iptables_rules_file);

    return true;
}

bool NetworkGateway::teardownIptables(const std::string &ipAddress)
{
    const char *iptables_command = "iptables -n -L FORWARD";
    FILE *fp = NULL;
    int line_no = -1; /* banner takes two lines. Start at 1 */
    char iptables_line[2048];

    fp = popen(iptables_command, "r");
    if (fp == NULL) {
        log_error("Error executing: %s", iptables_command);
        return false;
    }

    while (fgets(iptables_line, sizeof(iptables_line) - 1, fp) != NULL) {
        if (strstr(iptables_line, ipAddress.c_str()) != NULL) {
            char ipt_cmd[100];
            log_debug("%d > ", line_no);

            /* Actual deletion */
            snprintf(ipt_cmd, sizeof(ipt_cmd), "iptables -D FORWARD %d", line_no);
            if (system(ipt_cmd) == -1)
                log_error ("Failed to execute '%s'", ipt_cmd);
            /* We'll continue trying with the rest */

            line_no--; /* Removing this rule offsets the rest */
        }

        /* Print entire table */
        log_debug("%s", iptables_line);

        line_no++;
    }
    pclose(fp);

    return true;
}

std::string NetworkGateway::parseConfig(const std::string &config, const std::string &key) {
    json_error_t  error;
    json_t       *root, *value;

    /* Get root JSON object */
    root = json_loads(config.c_str(), 0, &error);

    if (!root) {
	log_error("Error on line %d: %s\n", error.line, error.text);
	goto cleanup_parse_json;
    }

    log_debug("Gateway configuration is: %s\n", json_dumps(root, 0));

    // Get string
    value = json_object_get(root, key.c_str());

    if (!json_is_string(value)) {
	log_error("Value is not a string.");
	log_error("error: on line %d: %s\n", error.line, error.text);
	json_decref (value);
	goto cleanup_parse_json;
    }

    log_debug("Value for %s is: %s\n", key.c_str(), json_string_value(value));

    return std::string(json_string_value(value));

cleanup_parse_json:
    return "";
}
