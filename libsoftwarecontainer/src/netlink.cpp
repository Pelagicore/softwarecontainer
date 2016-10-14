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
    m_sequenceNumber = 1;
    m_hasKernelDump = false;
    m_netlinkInitialized = false;

    if (isError(setupNetlink())) {
        fprintf(stderr, "Failed to setup netlink\n");
    }

    if (isError(getKernelDump())) {
        fprintf(stderr, "Failed to initialize cache\n");
    }
}

Netlink::~Netlink()
{
    if (m_netlinkInitialized) {
        shutdown(m_fd, SHUT_RDWR);
        close(m_fd);
    }

    if (m_hasKernelDump) {
        clearCache();
    }
}

ReturnCode Netlink::setupNetlink()
{
    if (m_netlinkInitialized) {
        return ReturnCode::SUCCESS;
    }

    if ((m_fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE)) == -1) {
        fprintf(stderr, "Socket error: %s", strerror(errno));
        return ReturnCode::FAILURE;
    }

    // Setup local sockaddr
    memset(&m_local, 0, sizeof(m_local));
    m_local.nl_family = AF_NETLINK;
    m_local.nl_groups = 0;

    if (bind(m_fd, (struct sockaddr *) &m_local, sizeof(m_local)) < 0) {
        fprintf(stderr, "Failed to bind socket: %s \n", strerror(errno));
        return ReturnCode::FAILURE;
    } else {
        m_pid = m_local.nl_pid;
    }

    // Setup kernel sockaddr
    memset(&m_kernel, 0, sizeof(m_kernel));
    m_kernel.nl_family = AF_NETLINK;

    m_netlinkInitialized = true;
    return ReturnCode::SUCCESS;
}

template<typename payload>
Netlink::netlink_request<payload> Netlink::createMessage(const int type, const int flags)
{
    netlink_request<payload> request;
    // Initialize length to be header + payload (no attributes)
    request.hdr.nlmsg_len = NLMSG_LENGTH(sizeof(payload));
    request.hdr.nlmsg_type = type;
    // We want ACK
    request.hdr.nlmsg_flags = flags | NLM_F_ACK | NLM_F_REQUEST;
    // Sequence numbers are monotonically increasing, pid is just an identifier
    request.hdr.nlmsg_seq = m_sequenceNumber++;
    request.hdr.nlmsg_pid = m_pid;
    // Set both payload and attribute buffer to 0
    memset(&request.pay, 0, sizeof(payload));
    memset(request.attr, 0, sizeof(request.attr));

    return request;
}

template<typename payload>
ReturnCode Netlink::addAttribute(netlink_request<payload> &request, const int type, const size_t length, const void *data)
{
    long unsigned int MAXSIZE = sizeof(request.attr);
    int usedAttributeLength = request.hdr.nlmsg_len - sizeof(request.hdr) - sizeof(request.pay);
    if (usedAttributeLength + RTA_LENGTH(length) > MAXSIZE) {
        return ReturnCode::FAILURE;
    }

    // First, get a pointer to memory at the end of the payload (pointed out by nlmsg_len)
    struct rtattr *rta = (struct rtattr *)(((char *) &request) + NLMSG_ALIGN(request.hdr.nlmsg_len));

    // Set type of attribute and attribute length (calculated with macro to include header)
    rta->rta_type = type;
    rta->rta_len = RTA_LENGTH(length);

    // Copy the data to the data location in the attribute
    memcpy(RTA_DATA(rta), data, length);

    // Update nlmsg_len to reflect that end of msg now includes an attribute
    request.hdr.nlmsg_len = NLMSG_ALIGN(request.hdr.nlmsg_len) + RTA_LENGTH(length);

    return ReturnCode::SUCCESS;
}

template<typename payload>
ReturnCode Netlink::sendMessage(netlink_request<payload> &request)
{
    if (!m_netlinkInitialized && isError(setupNetlink())) {
        fprintf(stderr, "Could not setup netlink communication with kernel\n");
        return ReturnCode::FAILURE;
    }

    // Create iovector, which holds pointer to, and size of,
    // the netlink message payload
    struct iovec io;
    memset(&io, 0, sizeof(io));
    io.iov_base = &request;
    io.iov_len = request.hdr.nlmsg_len;

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
        return ReturnCode::FAILURE;
    }

    // TODO: Distinguish from kernel errors (negative) and other errors (positive)
    int status = readMessage();
    if (status < 0) {
        fprintf(stderr, "Got an error from the kernel: %s\n", strerror(-status));
        return ReturnCode::FAILURE;
    } else if (status > 0) {
        fprintf(stderr, "Error in the netlink code: %i\n", status);
    }

    return ReturnCode::SUCCESS;
}

ReturnCode Netlink::setDefaultGateway(const char *gatewayAddress)
{
    netlink_request<rtmsg> set_gw = createMessage<rtmsg>(RTM_NEWROUTE, NLM_F_CREATE | NLM_F_REPLACE);
    set_gw.pay.rtm_family = AF_INET;
    set_gw.pay.rtm_table = RT_TABLE_MAIN;
    set_gw.pay.rtm_protocol = RTPROT_STATIC;
    set_gw.pay.rtm_scope = RT_SCOPE_UNIVERSE;
    set_gw.pay.rtm_type = RTN_UNICAST;

    struct in_addr gw_addr;
    if (inet_aton(gatewayAddress, &gw_addr) == 0) {
        return ReturnCode::FAILURE;
    }
    addAttribute(set_gw, RTA_GATEWAY, sizeof(gw_addr), &gw_addr);

    return sendMessage(set_gw);
}

// Bring up by index
ReturnCode Netlink::linkUp(const int ifaceIndex)
{
    if (isError(checkKernelDump())) {
        return ReturnCode::FAILURE;
    }

    for (LinkInfo link : m_links) {
        ifinfomsg ifinfo = link.first;
        if (ifinfo.ifi_index != ifaceIndex) {
            continue;
        }

        // First, bring the link up
        netlink_request<ifinfomsg> msg_up = createMessage<ifinfomsg>(RTM_NEWLINK, NLM_F_CREATE);
        msg_up.pay.ifi_family = AF_UNSPEC;
        msg_up.pay.ifi_flags = ifinfo.ifi_flags | IFF_UP;
        msg_up.pay.ifi_change |= IFF_UP;
        msg_up.pay.ifi_index = ifinfo.ifi_index;
        if (isError(sendMessage(msg_up))) {
            fprintf(stderr, "Failed to bring device %i up\n", ifinfo.ifi_index);
            return ReturnCode::FAILURE;
        } else {
            return ReturnCode::SUCCESS;
        }
    }

    return ReturnCode::FAILURE;
}

ReturnCode Netlink::setIP(const int ifaceIndex, const in_addr ip, const int netmask)
{
    for (LinkInfo link : m_links) {
        ifinfomsg ifinfo = link.first;
        if (ifinfo.ifi_index != ifaceIndex) {
            continue;
        }

        // TODO: Support for ipv6
        netlink_request<ifaddrmsg> msg_setip = createMessage<ifaddrmsg>(RTM_NEWADDR, NLM_F_CREATE | NLM_F_REPLACE);
        msg_setip.pay.ifa_family = AF_INET; // ipv4
        msg_setip.pay.ifa_prefixlen = netmask;
        msg_setip.pay.ifa_scope = RT_SCOPE_UNIVERSE;
        msg_setip.pay.ifa_index = ifinfo.ifi_index; // interface (link)

        // Calculate broadcast address from ip address
        // TODO: Use the netmask instead?!
        in_addr_t netpart = inet_netof(ip);
        struct in_addr bcast_addr = inet_makeaddr(netpart, inet_addr("0.0.0.255"));

        addAttribute(msg_setip, IFA_LOCAL, sizeof(ip), &ip);
        addAttribute(msg_setip, IFA_BROADCAST, sizeof(bcast_addr), &bcast_addr);

        // No matter what happens, we return since we found the link we've been looking for.
        if (isError(sendMessage(msg_setip))) {
            // TODO: pton to print ip number also.
            fprintf(stderr, "Failed to set ip on link %i\n", ifinfo.ifi_index);
            return ReturnCode::FAILURE;
        } else {
            return ReturnCode::SUCCESS;
        }
    }

    // No link found, that's a failure.
    return ReturnCode::FAILURE;
}

ReturnCode Netlink::linkDown(const int ifaceIndex)
{
    for (LinkInfo link : m_links) {
        ifinfomsg ifinfo = link.first;
        if(ifinfo.ifi_type == ARPHRD_LOOPBACK) {
            continue; // This is the loopback device
        }

        netlink_request<ifinfomsg> down_msg = createMessage<ifinfomsg>(RTM_NEWLINK, 0);
        down_msg.pay.ifi_family = AF_UNSPEC;
        down_msg.pay.ifi_index = ifinfo.ifi_index;
        down_msg.pay.ifi_flags = ~IFF_UP;
        down_msg.pay.ifi_change = IFF_UP;
        
        if (isError(sendMessage(down_msg))) {
            return ReturnCode::FAILURE;
        } else {
            return ReturnCode::SUCCESS;
        }
    }

    return ReturnCode::FAILURE;
}

ReturnCode Netlink::findLink(const char *ifaceName, LinkInfo &linkInfo)
{
    if (isError(checkKernelDump())) {
        return ReturnCode::FAILURE;
    }

    for (LinkInfo link : m_links) {
        AttributeList attributes = link.second;
        for (AttributeInfo attrinfo : attributes) {
            rtattr attr = attrinfo.first;
            void *data = attrinfo.second;

            if (attr.rta_type == IFLA_IFNAME) {
                char *ifname = (char *) data;
                if (strcmp(ifname, ifaceName) == 0) {
                    linkInfo = link;
                    return ReturnCode::SUCCESS;
                }
            }
        }
    }

    return ReturnCode::FAILURE;
}

ReturnCode Netlink::findAddresses(const unsigned int interfaceIndex, std::vector<AddressInfo> &result)
{
    if (isError(checkKernelDump())) {
        return ReturnCode::FAILURE;
    }

    for (AddressInfo addressInfo : m_addresses) {
        ifaddrmsg addrmsg = addressInfo.first;
        if (addrmsg.ifa_index == interfaceIndex) {
            result.push_back(addressInfo);
        }
    }

    return ReturnCode::SUCCESS;

}

ReturnCode Netlink::hasAddress(const std::vector<AddressInfo> &haystack, const int addressFamily, const char *needle)
{
    for (AddressInfo addressInfo : haystack) {
        AttributeList attributes = addressInfo.second;
        for (AttributeInfo attrPair : attributes) {

            // Skip if this attribute is not an address attribute
            rtattr attr = attrPair.first;
            if (attr.rta_type != IFA_ADDRESS && attr.rta_type != IFA_LOCAL) {
                continue;
            }

            void *data = attrPair.second;
            char out[INET6_ADDRSTRLEN];

            // It is ok here for inet_ntop to fail (data could be bad)
            if (inet_ntop(addressFamily, data, out, sizeof(out)) && strcmp(out, needle) == 0) {
                return ReturnCode::SUCCESS;
            }
        }
    }

    return ReturnCode::FAILURE;
}

int Netlink::readMessage()
{
    if (!m_netlinkInitialized && isError(setupNetlink())) {
        fprintf(stderr, "Could not setup netlink communication with kernel\n");
        return 1;
    }

    size_t PAGE_SIZE = getpagesize();
    char *buf= new char[PAGE_SIZE];

    bool end = false;
    while (!end) {
        int len;
        struct nlmsghdr *msg;
        struct msghdr reply;
        memset(&reply, 0, sizeof(reply));

        struct iovec io = { buf, sizeof(char) * PAGE_SIZE };
        reply.msg_iov = &io;
        reply.msg_iovlen = 1;
        reply.msg_name = &m_kernel;
        reply.msg_namelen = sizeof(m_kernel);

        len = recvmsg(m_fd, &reply, 0); /* read lots of data */
        if (len == 0) {
            end = true;
        }

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
                case NLMSG_DONE: // End of multipart message
                    end = true;
                    break;
                case RTM_GETLINK:
                case RTM_NEWLINK:
                    saveMessage<ifinfomsg, LinkInfo>(*msg, m_links);
                    break;
                case RTM_NEWADDR:
                case RTM_GETADDR:
                    saveMessage<ifaddrmsg, AddressInfo>(*msg, m_addresses);
                    break;
                case RTM_NEWROUTE:
                case RTM_GETROUTE:
                    saveMessage<rtmsg, RouteInfo>(*msg, m_routes);
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
    delete buf;
    return 0;
}

/**
 * To get a dump of all links/addresses/routes we only need to use the generic
 * netlink family message type, and for that we only need to set family (AF_PACKET).
 */
ReturnCode Netlink::getKernelDump()
{
    m_hasKernelDump = false;

    netlink_request<rtgenmsg> link_msg = createMessage<rtgenmsg>(RTM_GETLINK, NLM_F_DUMP);
    link_msg.pay.rtgen_family = AF_PACKET;
    if (isError(sendMessage(link_msg))) {
        return ReturnCode::FAILURE;
    }

    netlink_request<rtgenmsg> addr_msg = createMessage<rtgenmsg>(RTM_GETADDR, NLM_F_DUMP);
    addr_msg.pay.rtgen_family = AF_PACKET;
    if (isError(sendMessage(addr_msg))) {
        return ReturnCode::FAILURE;
    }

    netlink_request<rtgenmsg> route_msg = createMessage<rtgenmsg>(RTM_GETROUTE, NLM_F_DUMP);
    route_msg.pay.rtgen_family = AF_PACKET;
    if (isError(sendMessage(route_msg))) {
        return ReturnCode::FAILURE;
    }

    m_hasKernelDump = true;
    return ReturnCode::SUCCESS;
}

ReturnCode Netlink::checkKernelDump()
{
    if (!m_hasKernelDump && isError(getKernelDump())) {
        fprintf(stderr, "Could not get cache dump from kernel\n");
        return ReturnCode::FAILURE;
    }

    return ReturnCode::SUCCESS;
}

template<typename msgtype>
ReturnCode Netlink::getAttributes(const struct nlmsghdr &header, AttributeList &result)
{
    // Get the pointer to the data section of the message
    char *dataptr = (char *)NLMSG_DATA(&header);
    // Add on the size of the message type header = go to the data section
    // where attributes are stored = first attribute
    rtattr *attribute = (struct rtattr *)(dataptr + NLMSG_ALIGN(sizeof(msgtype)));

    // Total length of msg minus header = all attributes
    int len = header.nlmsg_len - NLMSG_LENGTH(sizeof(msgtype));
    while (RTA_OK(attribute, len)) {
        // Allocate enough data, and copy the attribute data to it
        void *data = malloc(RTA_PAYLOAD(attribute));
        if (data == nullptr) {
            fprintf(stderr, "Could not malloc enough to store attribute\n");
            return ReturnCode::FAILURE;
        }

        memcpy(data, RTA_DATA(attribute), RTA_PAYLOAD(attribute));
        // Then save the attribute and its data
        std::pair<rtattr, void *> pair(*attribute, data);
        result.push_back(pair);

        // Update attribute pointer
        attribute = RTA_NEXT(attribute, len);
    }
    
    return ReturnCode::SUCCESS;
}

template<typename msgtype, typename InfoType>
ReturnCode Netlink::saveMessage(const struct nlmsghdr &header, std::vector<InfoType> &result)
{
    // Bring out the msgtype struct
    msgtype *msg = (msgtype *) NLMSG_DATA(&header);
    // Get a vector of attributes for this message
    AttributeList attributes;
    if (isError(getAttributes<msgtype>(header, attributes))) {
        fprintf(stderr, "Could not get attributes for message\n");
        return ReturnCode::FAILURE;
    }
    // Save them both as a pair to the given result vector
    result.push_back(std::pair<msgtype, AttributeList>(*msg, attributes));
    return ReturnCode::SUCCESS;
}

void Netlink::freeAttributes(AttributeList &attrList)
{
    for (AttributeInfo attrInfo : attrList) {
        void *data = attrInfo.second;
        free(data);
    }
}

void Netlink::clearCache()
{
    for (LinkInfo link : m_links) {
        freeAttributes(link.second);
    }
    m_links.clear();

    for (AddressInfo addr : m_addresses) {
        freeAttributes(addr.second);
    }
    m_addresses.clear();

    for (RouteInfo route : m_routes) {
        freeAttributes(route.second);
    }
    m_routes.clear();

    m_hasKernelDump = false;
}
