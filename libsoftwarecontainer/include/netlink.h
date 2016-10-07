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
 *
 */

// ReturnCode
#include "softwarecontainer-common.h"

#include <linux/rtnetlink.h>
#include <arpa/inet.h>

#include <string>
#include <vector>

/*
 * @brief Handles various network operations over netlink
 *
 * Some of the code in this class is based on code shown here, written by Jean Lorchat
 * http://iijean.blogspot.se/2010/03/howto-get-list-of-network-interfaces-in.html
 */
class Netlink
{
    public:
        typedef std::vector<std::pair<int, std::string>> InterfaceList;

        /*
         * @brief Construct a new Netlink object
         *
         * This will do a setup of the sockets needed to talk netlink to the kernel,
         * and will also call getKernelDump()
         */
        Netlink();

        /*
         * @brief release all resources held by the Netlink object
         *
         * Since we allocate everything on the stack, except for the netlink attributes
         * which we save as void pointers, we basically go through all data structures
         * and free those pointers.
         *
         * Also shuts down the communication with the kernel.
         */
        ~Netlink();

        /*
         * @brief get a dump of all links, addresses and routes from the kernel.
         *
         * This is done by sending RTM_GETLINK, RTM_GETADDR and RTM_GETROUTE
         * as a general message (rtgenmsg) to the kernel.
         *
         * After running this successfully, there will be a local cache of these
         * network objects, which is needed to run some of the other functions.
         *
         * @return ReturnCode::SUCCESS on success, ReturnCode::FAILURE otherwise
         */
        ReturnCode getKernelDump();

        /*
         * @brief check for a kernel dump, and if not present, try to get one
         *
         * @return ReturnCode::SUCCESS if there a kernel dump was found or could be fetched
         * @return ReturnCode::FAILURE otherwise
         */
        ReturnCode checkKernelDump();

        /*
         * @brief Get a list of all network interfaces (except lo)
         *
         * Runs through the list and gives a list of all network interface namees
         * and indexes available.
         *
         * TODO: Check for each iface that we do have a name to add
         *
         * @return ReturnCode::SUCCESS if the operation was successful
         * @return ReturnCode::FAILURE if there was an error
         * @pre a local cache has to exist. If not, getKernelDump() will be run
         */
        ReturnCode getInterfaces(InterfaceList &result);

        /*
         * @brief bring an interface up and set its ip address
         *
         * This is similar to running: ifconfig <iface> up <ip> netmask <netmask>
         *
         * @param ifaceIndex the index of the interface
         * @param ip the ip address to set, in binary form
         * @parblock
         *      netmask the netmask for the network, as used in CIDR notation
         *      (for example 24 for /24)
         * @endparblock
         * @return ReturnCode::SUCCESS on success, ReturnCode::FAILURE otherwise
         */
        ReturnCode up(int ifaceIndex, in_addr ip, int netmask);

        /*
         * @brief bring an interface down
         *
         * This is similar to running ifconfig <iface> down
         *
         * @param ifaceIndex the index of the interface
         * @return ReturnCode::SUCCESS on success, ReturnCode::FAILURE otherwise
         */
        ReturnCode down(int ifaceIndex);

        /*
         * @brief checks if a given bridge is available with a given address
         *
         * @param bridgeName the name of the bridge interface
         * @param expectedAddress the expected address of the bridge, in dotted notation
         *
         * @return ReturnCode::SUCCESS if the bridge exists, is a bridge and has the expected address
         * @return ReturnCode::FAILURE otherwise
         *
         * TODO: Check if device is actually a bridge device
         */
        ReturnCode isBridgeAvailable(const char *bridgeName, const char *expectedAddress);

        /*
         * Sets an ip address as the default gateway
         *
         * @param gatewayAddress the address to set
         * @return ReturnCode::SUCCESS on success, ReturnCode::FAILURE otherwise
         */
        ReturnCode setDefaultGateway(const char *gatewayAddress);

    private:
        typedef std::pair<rtattr, void*> AttributeInfo;
        typedef std::vector<AttributeInfo> AttributeList;
        typedef std::pair<ifinfomsg, AttributeList> LinkInfo;
        typedef std::pair<ifaddrmsg, AttributeList> AddressInfo;
        typedef std::pair<rtmsg, AttributeList> RouteInfo;

        /*
         * @brief Save the netlink message contents in the given result vector
         *
         * @param h the netlink message header
         * @param result the vector in which to store the results
         * @return ReturnCode::SUCCESS on success, ReturnCode::FAILURE otherwise
         */
        template<typename msgtype, typename InfoType>
            ReturnCode saveMessage(struct nlmsghdr *header, std::vector<InfoType> &result);

        /*
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
            ReturnCode getAttributes(struct nlmsghdr *header, AttributeList &result);

        /*
         * @brief sets up the communication with the kernel over netlink
         *
         * Creates a socket and binds it
         *
         * @return ReturnCode::SUCCESS on success, ReturnCode::FAILURE otherwise
         */
        ReturnCode setupNetlink();

        /*
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

        /*
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
            netlink_request<payload> createMessage(int type, int flags);

        /*
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
            ReturnCode addAttribute(netlink_request<payload> &req, int type, size_t length, void *data);

        /*
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

        /*
         * @brief Listen for a netlink message
         *
         * This method will block while waiting to recieve a netlink message from the kernel.
         *
         * @return 0 if the read message was error-free, an error code otherwise
         */
        int readMessage();

        /*
         * @brief clears the cache
         */
        void clearCache();

        /*
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

