add_definitions(-DLXCTEMPLATE="${LXC_TEMPLATE_DIR}/lxc-softwarecontainer")

SET(LIBPELAGICONTAIN_DIR ${CMAKE_CURRENT_SOURCE_DIR})
add_subdirectory(include)
add_subdirectory(src)

if(ENABLE_TEST)
	add_subdirectory(unit-test)
endif(ENABLE_TEST)

option(ENABLE_NETWORK_GATEWAY "Enable network support" ON)
if(ENABLE_NETWORK_GATEWAY)
  file(READ ${PROJECT_SOURCE_DIR}/libsoftwarecontainer/softwarecontainer_network.conf.in NETWORK_LXC_CONF)
endif(ENABLE_NETWORK_GATEWAY)

configure_file(
  ${PROJECT_SOURCE_DIR}/libsoftwarecontainer/softwarecontainer.conf.in
  ${PROJECT_SOURCE_DIR}/libsoftwarecontainer/softwarecontainer.conf
)

configure_file(
	${PROJECT_SOURCE_DIR}/libsoftwarecontainer/lxc-softwarecontainer.in
	${CMAKE_BINARY_DIR}/libsoftwarecontainer/lxc-softwarecontainer
)

install(PROGRAMS ${CMAKE_BINARY_DIR}/libsoftwarecontainer/lxc-softwarecontainer DESTINATION ${LXC_TEMPLATE_DIR})
install(FILES softwarecontainer.conf DESTINATION ${SYS_CONFIG_DIR})
