/*
 * Copyright (C) 2016 Pelagicore AB
 *
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
 * WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR
 * BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES
 * OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
 * ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 *
 * For further information see LICENSE
 */

#include "netlink.h"

#include <linux/if_arp.h> // ARPHRD_ defines

#include <unistd.h> // getpid(), getpagesize()
#include <string.h> // memcpy, memset etc

Netlink::Netlink()
{
    m_sequence_number = 1;
    m_cache_dumped = false;
    m_netlink_initialized = false;

    if (!setup_netlink()) {
        fprintf(stderr, "Failed to setup netlink\n");
    }

    if (!get_dump()) {
        fprintf(stderr, "Failed to initialize cache\n");
        m_cache_dumped = false;
    } else {
        m_cache_dumped = true;
    }
}

Netlink::~Netlink()
{
    if (m_netlink_initialized) {
        shutdown(m_fd, SHUT_RDWR);
        close(m_fd);
    }

    if (m_cache_dumped) {
        clear_cache();
    }
}

bool Netlink::setup_netlink()
{
    if (m_netlink_initialized) {
        return true;
    }

    if ((m_fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE)) == -1) {
        fprintf(stderr, "Socket error: %s", strerror(errno));
        return false;
    }

    // Setup local sockaddr
    memset(&m_local, 0, sizeof(m_local));
    m_local.nl_family = AF_NETLINK;
    m_local.nl_groups = 0;

    if (bind(m_fd, (struct sockaddr *) &m_local, sizeof(m_local)) < 0) {
        fprintf(stderr, "Failed to bind socket: %s \n", strerror(errno));
        return false;
    } else {
        m_pid = m_local.nl_pid;
    }

    // Setup kernel sockaddr
    memset(&m_kernel, 0, sizeof(m_kernel));
    m_kernel.nl_family = AF_NETLINK; /*  fill-in kernel address (destination) */

    m_netlink_initialized = true;
    return true;
}

template<typename payload> Netlink::netlink_request<payload> Netlink::alloc_msg(const int type, const int flags)
{
    netlink_request<payload> request;
    // Initialize length to be header + payload (no attributes)
    request.hdr.nlmsg_len = NLMSG_LENGTH(sizeof(payload));
    request.hdr.nlmsg_type = type;
    // We want ACK
    request.hdr.nlmsg_flags = flags | NLM_F_ACK | NLM_F_REQUEST;
    // Sequence numbers are monotonically increasing, pid is just an identifier
    request.hdr.nlmsg_seq = m_sequence_number++;
    request.hdr.nlmsg_pid = m_pid;
    // Set both payload and attribute buffer to 0
    memset(&request.pay, 0, sizeof(payload));
    memset(request.attr, 0, sizeof(request.attr));

    return request;
}

template<typename payload>
void Netlink::add_attribute(netlink_request<payload> &req, int type, size_t length, void *data)
{
    // First, get a pointer to memory at the end of the payload (pointed out by nlmsg_len)
    struct rtattr *rta = (struct rtattr *)(((char *) &req) + NLMSG_ALIGN(req.hdr.nlmsg_len));

    // Set type of attribute and attribute length (calculated with macro to include header)
    rta->rta_type = type;
    rta->rta_len = RTA_LENGTH(length);

    // Copy the data to the data location in the attribute
    memcpy(RTA_DATA(rta), data, length);

    // Update nlmsg_len to reflect that end of msg now includes an attribute
    req.hdr.nlmsg_len = NLMSG_ALIGN(req.hdr.nlmsg_len) + RTA_LENGTH(length);
}

template<typename payload>
bool Netlink::send_msg(netlink_request<payload> req)
{
    // Create iovector, which holds pointer to, and size of,
    // the netlink message payload
    struct iovec io;
    memset(&io, 0, sizeof(io));
    io.iov_base = &req;
    io.iov_len = req.hdr.nlmsg_len;

    // Create the message header, to wrap everything in
    struct msghdr msghdr;
    memset(&msghdr, 0, sizeof(msghdr));

    // We only send one message here, and it's the one we set in the vector above
    msghdr.msg_iov = &io;
    msghdr.msg_iovlen = 1;
    // We send it to the kernel
    msghdr.msg_name = &m_kernel;
    msghdr.msg_namelen = sizeof(m_kernel);

    if ((sendmsg(m_fd, &msghdr, 0)) == -1) {
        fprintf(stderr, "Unable to send msg to kernel: %s\n", strerror(errno));
        return false;
    }

    if (read_msg() != 0) {
        fprintf(stderr, "Got an error from the kernel\n");
        return false;
    }

    return true;
}

bool Netlink::setDefaultGateway(const char *gateway_address)
{
    netlink_request<rtmsg> set_gw = alloc_msg<rtmsg>(RTM_NEWROUTE, NLM_F_CREATE | NLM_F_REPLACE);
    set_gw.pay.rtm_family = AF_INET;
    set_gw.pay.rtm_table = RT_TABLE_MAIN;
    set_gw.pay.rtm_protocol = RTPROT_STATIC;
    set_gw.pay.rtm_scope = RT_SCOPE_UNIVERSE;
    set_gw.pay.rtm_type = RTN_UNICAST;

    struct in_addr gw_addr;
    if (inet_aton(gateway_address, &gw_addr) == 0) {
        return false;
    }
    add_attribute(set_gw, RTA_GATEWAY, sizeof(gw_addr), &gw_addr);

    return send_msg(set_gw);
}

// Bring up by index
bool Netlink::up(int iface_index, in_addr ip, int netmask)
{
    if (!m_cache_dumped && !get_dump()) {
        fprintf(stderr, "Could not get cache dump from kernel\n");
        return false;
    }

    for (LinkInfo link : m_links) {
        ifinfomsg ifinfo = link.first;
        if (ifinfo.ifi_index != iface_index) {
            continue;
        }

        // First, bring the link up
        netlink_request<ifinfomsg> msg_up = alloc_msg<ifinfomsg>(RTM_NEWLINK, NLM_F_CREATE);
        msg_up.pay.ifi_family = AF_UNSPEC;
        msg_up.pay.ifi_flags = ifinfo.ifi_flags | IFF_UP;
        msg_up.pay.ifi_change |= IFF_UP;
        msg_up.pay.ifi_index = ifinfo.ifi_index;
        if (!send_msg(msg_up)) {
            fprintf(stderr, "Failed to bring device %i up\n", ifinfo.ifi_index);
            return false;
        }

        // If this is the loopback device, we can't set an IP address for it
        // and bringing it up is enough. Continue to next link.
        if(ifinfo.ifi_type == ARPHRD_LOOPBACK) {
            continue;
        }

        // Second, set IP address
        // TODO: Support for ipv6
        netlink_request<ifaddrmsg> msg_setip = alloc_msg<ifaddrmsg>(RTM_NEWADDR, NLM_F_CREATE | NLM_F_REPLACE);
        msg_setip.pay.ifa_family = AF_INET; // ipv4
        msg_setip.pay.ifa_prefixlen = netmask;
        msg_setip.pay.ifa_scope = RT_SCOPE_UNIVERSE;
        msg_setip.pay.ifa_index = ifinfo.ifi_index; // interface (link)

        // Calculate broadcast address from ip address
        // TODO: Use the netmask instead?!
        in_addr_t netpart = inet_netof(ip);
        struct in_addr bcast_addr = inet_makeaddr(netpart, inet_addr("0.0.0.255"));

        add_attribute(msg_setip, IFA_LOCAL, sizeof(ip), &ip);
        add_attribute(msg_setip, IFA_BROADCAST, sizeof(bcast_addr), &bcast_addr);

        if (!send_msg(msg_setip)) {
            // TODO: pton to print ip number also.
            fprintf(stderr, "Failed to set ip on link %i\n", ifinfo.ifi_index);
            return false;
        }

        return true;
    }

    return false;
}

bool Netlink::down(int iface_index)
{
    if (!m_cache_dumped && !get_dump()) {
        fprintf(stderr, "Could not get cache dump from kernel\n");
        return false;
    }

    // Remove all known addresses for this interface
    for (AddressInfo addr : m_addresses) {
        ifaddrmsg ifaddr = addr.first;
        if (!ifaddr.ifa_index == iface_index) {
            continue;
        }

        netlink_request<ifaddrmsg> addr_msg = alloc_msg<ifaddrmsg>(RTM_DELLINK, 0);
        memcpy(&addr_msg.pay, &ifaddr, sizeof(ifaddrmsg));
        send_msg(addr_msg);
    }

    // Then, remove the actual links.
    for (LinkInfo link : m_links) {
        ifinfomsg ifinfo = link.first;
        if(ifinfo.ifi_type == ARPHRD_LOOPBACK) {
            continue; // This is the loopback device
        }

        netlink_request<ifinfomsg> down_msg = alloc_msg<ifinfomsg>(RTM_NEWLINK, 0);
        down_msg.pay.ifi_family = AF_UNSPEC;
        down_msg.pay.ifi_index = ifinfo.ifi_index;
        down_msg.pay.ifi_flags = ~IFF_UP;
        down_msg.pay.ifi_change = IFF_UP;
        send_msg(down_msg);
    }
    return true;

}

bool Netlink::isBridgeAvailable(const char *bridgeName, const char *expectedAddress)
{
    if (!m_cache_dumped && !get_dump()) {
        fprintf(stderr, "Could not get cache dump from kernel\n");
        return false;
    }

    bool hasBridge = false;
    unsigned int bridge_ifindex = -1;

    for (LinkInfo linkinfo : m_links) {
        AttributeList attributes = linkinfo.second;
        for (AttributeInfo attrinfo : attributes) {
            rtattr attr = attrinfo.first;
            void *data = attrinfo.second;

            if (attr.rta_type == IFLA_IFNAME) {
                char *ifname = (char *) data;
                if (strcmp(ifname, bridgeName) == 0) {
                    hasBridge = true;
                    bridge_ifindex = linkinfo.first.ifi_index;
                    break;
                }
            }
        }

        // TODO: Make this nicer, please
        if (hasBridge) {
            break;
        }
    }

    if (!hasBridge) {
        return false;
    }

    bool bridgeHasGateway = false;
    for (AddressInfo addressInfo : m_addresses) {
        ifaddrmsg addrmsg = addressInfo.first;
        if (addrmsg.ifa_index != bridge_ifindex) {
            continue;
        }

        AttributeList attributes = addressInfo.second;
        for (AttributeInfo attrPair : attributes) {
            rtattr attr = attrPair.first;
            if (attr.rta_type != IFA_ADDRESS && attr.rta_type != IFA_LOCAL) {
                continue;
            }

            void *data = attrPair.second;
            char out[INET6_ADDRSTRLEN];
            // It is ok here for inet_ntop to fail (data could be bad)
            if (inet_ntop(addrmsg.ifa_family, data, out, sizeof(out))) {
                if (strcmp(out, expectedAddress) == 0) {
                    bridgeHasGateway = true;
                    break;
                }
            }
        }

        if (bridgeHasGateway) {
            break;
        }
    }

    return bridgeHasGateway;
}

int Netlink::read_msg()
{
    bool end = false;
    size_t PAGE_SIZE = getpagesize();
    char buf[PAGE_SIZE];

    while (!end) {
        int len;
        struct nlmsghdr *msg;
        struct msghdr reply;
        memset(&reply, 0, sizeof(reply));

        struct iovec io = { buf, PAGE_SIZE };
        reply.msg_iov = &io;
        reply.msg_iovlen = 1;
        reply.msg_name = &m_kernel;
        reply.msg_namelen = sizeof(m_kernel);

        len = recvmsg(m_fd, &reply, 0); /* read lots of data */
        if (len > 0) {
            for (msg = (struct nlmsghdr *) buf; NLMSG_OK(msg, len); msg = NLMSG_NEXT(msg, len)) {
                switch(msg->nlmsg_type)
                {
                    case NLMSG_ERROR: {
                        struct nlmsgerr *err = (struct nlmsgerr *)NLMSG_DATA(msg);
                        if (err->error != 0) {
                            return err->error; // Actually an error
                        } else {
                            end = true; // This was an ack
                        }

                        break;
                    }
                    case NLMSG_DONE:
                        end = true;
                        break;
                    case RTM_GETLINK:
                    case RTM_NEWLINK:
                        save_msg<ifinfomsg, LinkInfo>(msg, m_links);
                        break;
                    case RTM_NEWADDR:
                    case RTM_GETADDR:
                        save_msg<ifaddrmsg, AddressInfo>(msg, m_addresses);
                        break;
                    case RTM_NEWROUTE:
                    case RTM_GETROUTE:
                        save_msg<rtmsg, RouteInfo>(msg, m_routes);
                        break;
                    case RTM_NEWNEIGH:
                    case RTM_GETNEIGH:
                        break;
                    case RTM_NEWRULE:
                    case RTM_GETRULE:
                        break;
                    case RTM_NEWQDISC:
                    case RTM_GETQDISC:
                        break;
                    case RTM_NEWTCLASS:
                    case RTM_GETTCLASS:
                        break;
                    case RTM_NEWTFILTER:
                    case RTM_GETTFILTER:
                        break;
                    // Group all delete cases together, since we don't want to
                    // save anything here. TODO: Delete if present in cache
                    case RTM_DELLINK:
                        printf("Got dellink\n");
                        break;
                    case RTM_DELROUTE:
                    case RTM_DELADDR:
                    case RTM_DELTFILTER:
                    case RTM_DELTCLASS:
                    case RTM_DELQDISC:
                    case RTM_DELRULE:
                    case RTM_DELNEIGH:
                        break;

                    default:    /* for education only, should not happen here */
                        printf("message type %d, length %d\n", msg->nlmsg_type, msg->nlmsg_len);
                        break;
                }
            }
        }
    }
    return 0;
}

/**
 * To get a dump of all links/addresses/routes we only need to use the generic
 * netlink family message type, and for that we only need to set family (AF_PACKET).
 */
bool Netlink::get_dump()
{
    m_cache_dumped = false;

    netlink_request<rtgenmsg> link_msg = alloc_msg<rtgenmsg>(RTM_GETLINK, NLM_F_DUMP);
    link_msg.pay.rtgen_family = AF_PACKET;
    if (!send_msg(link_msg)) {
        return false;
    }

    netlink_request<rtgenmsg> addr_msg = alloc_msg<rtgenmsg>(RTM_GETADDR, NLM_F_DUMP);
    addr_msg.pay.rtgen_family = AF_PACKET;
    if (!send_msg(addr_msg)) {
        return false;
    }

    netlink_request<rtgenmsg> route_msg = alloc_msg<rtgenmsg>(RTM_GETROUTE, NLM_F_DUMP);
    route_msg.pay.rtgen_family = AF_PACKET;
    if (!send_msg(route_msg)) {
        return false;
    }

    m_cache_dumped = true;
    return true;
}

std::vector<std::pair<int, std::string>> Netlink::get_interfaces()
{
    std::vector<std::pair<int, std::string>> ifaces;

    if (!m_cache_dumped && !get_dump()) {
        fprintf(stderr, "Could not get cache dump from kernel\n");
        return ifaces;
    }

    for (LinkInfo link : m_links) {
        ifinfomsg ifinfo = link.first;
        if(ifinfo.ifi_type == ARPHRD_LOOPBACK) {
            continue; // We don't include loopback here
        }

        AttributeList attributeList = link.second;
        for (AttributeInfo attrInfo : attributeList) {
            rtattr attr = attrInfo.first;
            if (attr.rta_type == IFLA_IFNAME) {
                const char *name = (char *)attrInfo.second;

                std::pair<int, std::string> pair(ifinfo.ifi_index, std::string(name));
                ifaces.push_back(pair);
                break; // Just break out of this inner for
            }
        }
    }

    return ifaces;
}


template<typename msgtype> Netlink::AttributeList Netlink::get_attributes(struct nlmsghdr *h)
{
    std::vector<AttributeInfo> attributes;

    // Get the pointer to the data section of the message
    char *dataptr = (char *)NLMSG_DATA(h);
    // Add on the size of the message type header = go to the data section
    // where attributes are stored
    rtattr *attribute = (struct rtattr *)(dataptr + NLMSG_ALIGN(sizeof(msgtype)));

    // Total length of msg minus header = all attributes
    int len = h->nlmsg_len - NLMSG_LENGTH(sizeof(msgtype));
    do {
        // Allocate enough data, and copy the attribute data to it
        void *data = malloc(RTA_PAYLOAD(attribute));
        memcpy(data, RTA_DATA(attribute), RTA_PAYLOAD(attribute));
        // Then save the attribute and its data
        std::pair<rtattr, void *> pair(*attribute, data);
        attributes.push_back(pair);

        // Update attribute pointer
        attribute = RTA_NEXT(attribute, len);
    } while (RTA_OK(attribute, len));
    
    return attributes;
}

template<typename msgtype, typename InfoType>
void Netlink::save_msg(struct nlmsghdr *h, std::vector<InfoType> &result)
{
    // Bring out the msgtype struct
    msgtype *msg = (msgtype *) NLMSG_DATA(h);
    // Get a vector of attributes for this message
    std::vector<AttributeInfo> attributes = get_attributes<msgtype>(h);
    // Save them both as a pair to the given result vector
    result.push_back(std::pair<msgtype, AttributeList>(*msg, attributes));
}

bool Netlink::clear_cache()
{
    // TODO: Make sure we call free on all attributes here
    m_links.clear();
    m_addresses.clear();
    m_routes.clear();

    return true;
}
