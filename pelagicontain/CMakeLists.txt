cmake_minimum_required(VERSION 2.6)

add_definitions(-DLXCTEMPLATE="${LXC_TEMPLATE_DIR}/lxc-pelagicontain")

add_subdirectory(src)
add_subdirectory(doc)

if(BUILD_TESTS)
	add_subdirectory(unit-test)
	add_subdirectory(component-test)
endif(BUILD_TESTS)

install(PROGRAMS lxc-pelagicontain DESTINATION ${LXC_TEMPLATE_DIR})
install(FILES pelagicontain.conf DESTINATION ${SYS_CONFIG_DIR})
