/*
 * Copyright (C) 2016-2017 Pelagicore AB
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
 *
 */

#include "softwarecontainer-common.h"

#include <linux/rtnetlink.h>
#include <arpa/inet.h>

#include <string>
#include <vector>

namespace softwarecontainer {

/**
 * @brief Handles various network operations over netlink
 *
 * Some of the code in this class is based on code shown here, written by Jean Lorchat
 * http://iijean.blogspot.se/2010/03/howto-get-list-of-network-interfaces-in.html
 */
class Netlink
{
    public:
        typedef std::pair<rtattr, void*> AttributeInfo;
        typedef std::vector<AttributeInfo> AttributeList;
        typedef std::pair<ifinfomsg, AttributeList> LinkInfo;
        typedef std::pair<ifaddrmsg, AttributeList> AddressInfo;
        typedef std::pair<rtmsg, AttributeList> RouteInfo;

        /**
         * @brief Construct a new Netlink object
         *
         * This will do a setup of the sockets needed to talk netlink to the kernel,
         * and will also call getKernelDump()
         */
        Netlink();

        /**
         * @brief release all resources held by the Netlink object
         *
         * Since we allocate everything on the stack, except for the netlink attributes
         * which we save as void pointers, we basically go through all data structures
         * and free those pointers.
         *
         * Also shuts down the communication with the kernel.
         */
        virtual ~Netlink();

        /**
         * @brief get a dump of all links, addresses and routes from the kernel.
         *
         * This is done by sending RTM_GETLINK, RTM_GETADDR and RTM_GETROUTE
         * as a general message (rtgenmsg) to the kernel.
         *
         * After running this successfully, there will be a local cache of these
         * network objects, which is needed to run some of the other functions.
         *
         * @return ReturnCode::SUCCESS on success, ReturnCode::FAILURE otherwise
         *
         * @sa clearCache()
         */
        ReturnCode getKernelDump();

        /**
         * @brief check for a kernel dump, and if not present, try to get one
         *
         * @return ReturnCode::SUCCESS if there a kernel dump was found or could be fetched
         * @return ReturnCode::FAILURE otherwise
         */
        virtual ReturnCode checkKernelDump();

        /**
         * @brief Sets an ip address as the default gateway
         *
         * @param gatewayAddress the address to set
         * @return ReturnCode::SUCCESS on success
         * @return ReturnCode::FAILURE otherwise
         */
        ReturnCode setDefaultGateway(const char *gatewayAddress);

        /**
         * @brief Bring the given interface up
         *
         * Sets the UP flag for the given interface, if it exists.
         *
         * @param ifaceIndex the index for the interface to bring up
         * @return ReturnCode::SUCCESS if the interface was found and brought up
         * @return ReturnCode::FAILURE otherwise
         */
        ReturnCode linkUp(const int ifaceIndex);

        /**
         * @brief Bring a given interface down
         *
         * @param ifaceIndex the index for the interface to bring down
         * @return ReturnCode::SUCCESS if interface was found and brought down
         * @return ReturnCode::FAILURE otherwise
         */
        ReturnCode linkDown(const int ifaceIndex);

        /**
         * @brief Sets an IP address for a network link
         *
         * @param ifaceIndex the index for the interface to set ip for
         * @param ip the ipv4 address to set
         * @param netmask the netmask in CIDR format (for example 24)
         *
         * @return ReturnCode::SUCCESS if interface was found and IP was set
         * @return ReturnCode::FAILURE otherwise
         */
        ReturnCode setIP(const int ifaceIndex, const in_addr ip, const unsigned char netmask);

        /**
         * @brief Check that the device given is a network bridge
         * @param ifaceName the name of the interface
         * @param ifaceIndex the index of the interface (out parameter)
         *
         * @return ReturnCode::SUCCESS if interface with matching name was found.
         * @return ReturnCode::FAILURE otherwise.
         */
        ReturnCode findLink(const char *ifaceName, LinkInfo &linkInfo);

        /**
         * @brief Get all addresses associated with the given interface index
         *
         * @param interfaceIndex the interface address to get addresses for
         * @param result the vector to place all addresses in (out parameter)
         *
         * @return ReturnCode::SUCCESS on success
         * @return ReturnCode::FAILURE otherwise
         */
        ReturnCode findAddresses(const unsigned int interfaceIndex, std::vector<AddressInfo> &result);

        /**
         * @brief checks if an address is present in the given list
         *
         * @param haystack the list to search in
         * @param addressFamily the address family, AF_INET or AF_INET6
         * @param needle the ip address to search for, in dotted notation
         *
         * @return ReturnCode::SUCCESS if the address is in the haystack
         * @return ReturnCode::FAILURE otherwise
         */
        ReturnCode hasAddress(const std::vector<AddressInfo> &haystack, const int addressFamily, const char *needle);

    private:
        /**
         * @brief sets up the communication with the kernel over netlink
         *
         * Creates a socket and binds it
         *
         * @return ReturnCode::SUCCESS on success, ReturnCode::FAILURE otherwise
         */
        ReturnCode setupNetlink();

        /**
         * @brief clears the cache
         *
         * This clears all the links, addresses and routes. It also has the responsibility to free
         * all allocated attributes for links, addresses och routes.
         *
         * @sa getKernelDump()
         */
        void clearCache();

        /**
         * @brief Save the netlink message contents in the given result vector
         *
         * @param h the netlink message header
         * @param result the vector in which to store the results
         * @return ReturnCode::SUCCESS on success, ReturnCode::FAILURE otherwise
         */
        template<typename msgtype, typename InfoType>
            ReturnCode saveMessage(const struct nlmsghdr &header, std::vector<InfoType> &result);

        /**
         * @brief Save the attributes and attribute data from a netlink message
         *
         * This parses all attributes and saves the rtattr structure together with the
         * void pointers to the data as a pair in a vector. Since each attribute may
         * be of a different type, it is hard to parse out the data pointer to something
         * more reasonable.
         *
         * @note this function allocates memory on the heap. Use freeAttributes to free the list
         *
         * @param h the netlink message header
         * @param result a reference to the list in which to store results
         * @result ReturnCode::SUCCESS on success, ReturnCode::FAILURE otherwise
         */
        template<typename msgtype>
            ReturnCode getAttributes(const struct nlmsghdr &header, AttributeList &result);

        /**
         * Templatified general structure for netlink requests
         *
         * With the exception of the msghdr, this structure contains everything needed
         * to create a netlink message.
         *
         * Netlink messages consist of a header, a payload of a certain type, and optionally
         * a bunch of attributes, which in turn follow a Tag-Length-Value setup, so their
         * content is basically just raw data.
         *
         * @tparam payload the type of netlink message
         */
        template<typename payload>struct netlink_request {
            nlmsghdr hdr;
            payload pay;
            char attr[2048];
        };

        /**
         * @brief get a netlink request with given type and flags
         *
         * Creates an empty netlink_request struct with just some basic
         * fields set.
         *
         * @tparam payload the type of netlink message to create
         * @param type the type of netlink message
         * @param flags any flags to set in the message header
         * @return a new netlink_request with given payload type
         */
        template<typename payload>
            netlink_request<payload> createMessage(const int type, const int flags);

        /**
         * @brief adds an attribute to a netlink request
         *
         * Handling netlink attributes is HARD. We use a function to add them to avoid having
         * to write all netlink macro magic each time. Basically, this adds a rtattr structure
         * at the end of the occupied part of the attribute buffer in the request, and updates
         * the message length.
         *
         * @tparam payload the type of netlink message we're dealing with
         * @param req the netlink request
         * @param type attribute type
         * @param length attribute length
         * @param data attribute data
         *
         * @note the data param is copied. The pointer can be freed after this method returns.
         *
         * @return ReturnCode::SUCCESS if the attribute fits and was added.
         * @return ReturnCode::FAILURE otherwise
         */
        template<typename payload>
            ReturnCode addAttribute(netlink_request<payload> &req, const int type, const size_t length, const void *data);

        /**
         * @brief send a netlink message and check for reply
         *
         * This sends the given request to the kernel and parses the reply as well.
         *
         * @tparam payload the netlink message type
         * @param req the netlink request to send
         *
         * @return ReturnCode::SUCCESS if sending was successful and reply was error-free.
         * @return ReturnCode::FAILURE otherwise
         */
        template<typename payload>
            ReturnCode sendMessage(netlink_request<payload> &request);

        /**
         * @brief Listen for a netlink message
         *
         * This method will block while waiting to recieve a netlink message from the kernel.
         *
         * @return 0 if the read message was error-free, an error code otherwise
         */
        int readMessage();

        /**
         * @brief frees all attribute pointers saved in a list
         */
        void freeAttributes(AttributeList &attrList);

        // Netlink communication variables
        struct sockaddr_nl m_local;
        struct sockaddr_nl m_kernel;
        int m_fd;
        pid_t m_pid;
        unsigned int m_sequenceNumber;

        // The local cache of links, addresses and routes
        std::vector< LinkInfo > m_links;
        std::vector< AddressInfo > m_addresses;
        std::vector< RouteInfo > m_routes;

        // Status flags
        bool m_hasKernelDump;
        bool m_netlinkInitialized;
};

} // namespace softwarecontainer
