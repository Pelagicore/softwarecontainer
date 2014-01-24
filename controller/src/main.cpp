
#include <iostream>
#include <unistd.h>
#include "errno.h"



       #include <sys/types.h>
       #include <sys/stat.h>
       #include <fcntl.h>
int main(int argc, char **argv)
{
// 	DBus::BusDispatcher dispatcher;
// 	DBus::default_dispatcher = &dispatcher;
// 	DBus::Connection bus = DBus::Connection::SessionBus();
// 
// 	bus.request_name("com.pelagicore.Controller");
// 
// 	Controller controller;
// 	ControllerToDBusAdapter controllerAdapter(bus, controller);
// 
// 	dispatcher.enter();
// 
	std::cout << "In Controller" << std::endl;
	char c;
// 	for (int i = 0; i < 300; i++) {
// 		int status = read(0, &c, 1);
	int fd = open("/deployed_app/in_fifo", O_RDONLY);
	for (;;) {
		
		int status = read(fd, &c, 1);
		if (status > 0)
			std::cout << "read ststus is: " << status << c << errno << std::endl;
	}
// 		std::cout << "read ststus is: " << status << c << errno << std::endl;
// 	}
	return 0;
}
