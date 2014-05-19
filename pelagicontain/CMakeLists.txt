cmake_minimum_required(VERSION 2.6)

add_definitions(-DLXCTEMPLATE="${LXC_TEMPLATE_DIR}/lxc-pelagicontain")

add_subdirectory(src)

if(ENABLE_TEST)
	add_subdirectory(unit-test)
endif(ENABLE_TEST)

set(LXC_HOOK_SCRIPT_MAKESLAVE "${LXC_HOOK_SCRIPTS_DIR}/lxc-hook-makeslave.sh")
configure_file(
	${PROJECT_SOURCE_DIR}/pelagicontain/lxc-pelagicontain.in
	${CMAKE_BINARY_DIR}/pelagicontain/lxc-pelagicontain
)
install(PROGRAMS ${CMAKE_BINARY_DIR}/pelagicontain/lxc-pelagicontain DESTINATION ${LXC_TEMPLATE_DIR})
install(FILES pelagicontain.conf DESTINATION ${SYS_CONFIG_DIR})
install(PROGRAMS lxc-hook-makeslave.sh DESTINATION ${LXC_HOOK_SCRIPTS_DIR})
