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

#include <linux/rtnetlink.h>
#include <arpa/inet.h>

#include <string>
#include <vector>

/*
 * @brief Handles various network operations over netlink
 * TODO: Consistent naming of things
 *
 * Some of the code in this class is based on code shown here, written by Jean Lorchat
 * http://iijean.blogspot.se/2010/03/howto-get-list-of-network-interfaces-in.html
 */
class Netlink
{
    public:

        /*
         * @brief Construct a new Netlink object
         *
         * This will do a setup of the sockets needed to talk netlink to the kernel,
         * and will also call get_dump()
         */
        Netlink();

        /*
         * TODO: Do something reasonable here.
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
         * @return true on success, false otherwise
         */
        bool get_dump();

        /*
         * @brief Get a list of all network interfaces (except lo)
         *
         * Runs through the list and gives a list of all network interface namees
         * and indexes available.
         *
         * @return a vector of interfaces on success, an empty vector on failure // TODO: FIX THIS
         * @pre a local cache has to exist. If not, get_dump() will be run
         */
        std::vector<std::pair<int,std::string>> get_interfaces();

        /*
         * @brief bring an interface up and set its ip address
         *
         * This is similar to running: ifconfig <iface> up <ip> netmask <netmask>
         *
         * @param iface_index the index of the interface
         * @param ip the ip address to set, in binary form
         * @parblock
         *      netmask the netmask for the network, as used in CIDR notation
         *      (for example 24 for /24)
         * @endparblock
         * @return true on success, false otherwise
         */
        bool up(int iface_index, in_addr ip, int netmask);

        /*
         * @brief bring an interface down
         *
         * This is similar to running ifconfig <iface> down
         *
         * @param iface_index the index of the interface
         * @return true on success, false otherwise
         */
        bool down(int iface_index);

        /*
         * @brief checks if a given bridge is available with a given address
         *
         * @param bridgeName the name of the bridge interface
         * @param expectedAddress the expected address of the bridge, in dotted notation
         *
         * @return true if the bridge exists, is a bridge and has the expected address
         * @return false otherwise
         *
         * TODO: Check if device is actually a bridge device
         */
        bool isBridgeAvailable(const char *bridgeName, const char *expectedAddress);

        /*
         * Sets an ip address as the default gateway
         *
         * @param gateway_address the address to set
         * @return true on success, false otherwise
         */
        bool setDefaultGateway(const char *gateway_address);

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
         * TODO: What about errors here?
         */
        template<typename msgtype, typename InfoType> void save_msg(struct nlmsghdr *h, std::vector<InfoType> &result);

        /*
         * @brief Save the attributes and attribute data from a netlink message
         *
         * This parses all attributes and saves the rtattr structure together with the
         * void pointers to the data as a pair in a vector. Since each attribute may
         * be of a different type, it is hard to parse out the data pointer to something
         * more reasonable.
         *
         * @param h the netlink message header
         * @return an AttributeList with all attributes connected to this header
         * TODO: What about errors here?
         */
        template<typename msgtype> AttributeList get_attributes(struct nlmsghdr *h);

        /*
         * @brief sets up the communication with the kernel over netlink
         *
         * Creates a socket and binds it
         *
         * @return true on success, false otherwise
         */
        bool setup_netlink();

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
        template<typename payload> netlink_request<payload> alloc_msg(int type, int flags);

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
         * @return nothing TODO: WHAT?
         */
        template<typename payload> void add_attribute(netlink_request<payload> &req, int type, size_t length, void *data);

        /*
         * @brief send a netlink message and check for reply
         *
         * This sends the given request to the kernel and parses the reply as well.
         *
         * @tparam payload the netlink message type
         * @param req the netlink request to send
         *
         * @return true if sending was successful and reply was error-free, false otherwise
         */
        template<typename payload> bool send_msg(netlink_request<payload> req);

        /*
         * @brief Listen for a netlink message
         *
         * This method will block while waiting to recieve a netlink message from the kernel.
         *
         * @return 0 if the read message was error-free, an error code otherwise
         */
        int read_msg();

        /*
         * @brief clears the cache
         */
        bool clear_cache();

        struct sockaddr_nl m_local;
        struct sockaddr_nl m_kernel;
        int m_fd;
        pid_t m_pid;
        unsigned int m_sequence_number;

        // The local cache of links, addresses and routes
        std::vector< LinkInfo > m_links;
        std::vector< AddressInfo > m_addresses;
        std::vector< RouteInfo > m_routes;

        bool m_cache_dumped;
        bool m_netlink_initialized;
};

