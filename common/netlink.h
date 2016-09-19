#include <linux/rtnetlink.h>

#include <string>
#include <vector>

class Netlink
{
    public:

        Netlink();
        ~Netlink();
        bool get_dump();

        bool up();
        bool down();
        bool isBridgeAvailable();
        bool setDefaultGateway();
        bool generateIP();

        const std::string ip();

    private:
        typedef std::pair<rtattr, void*> AttributeInfo;
        typedef std::vector<AttributeInfo> AttributeList;
        typedef std::pair<ifinfomsg, AttributeList> LinkInfo;
        typedef std::pair<ifaddrmsg, AttributeList> AddressInfo;
        typedef std::pair<rtmsg, AttributeList> RouteInfo;

        template<typename msgtype, typename InfoType> void save_msg(struct nlmsghdr *h, std::vector<InfoType> &result);
        template<typename msgtype> AttributeList get_attributes(struct nlmsghdr *h);

        bool setup_netlink();
        
        template<typename payload>struct netlink_request {
            nlmsghdr hdr;
            payload pay;
            char attr[2048];
        };

        // Allocate a msg with given payload
		template<typename payload> void add_attribute(netlink_request<payload> &req, int type, size_t length, void *data);
		template<typename payload> netlink_request<payload> alloc_msg(int type, int flags);
		template<typename payload> bool send_msg(netlink_request<payload> req);
        int read_msg();

        struct sockaddr_nl m_local;
        struct sockaddr_nl m_kernel;
        int m_fd;
        pid_t m_pid;
        unsigned int m_sequence_number;

        std::vector< LinkInfo > m_links;
        std::vector< AddressInfo > m_addresses;
        std::vector< RouteInfo > m_routes;

        std::string m_ip;
		std::string m_gateway;
        std::string m_broadcast;
        bool m_interfaceInitialized = false;

};

