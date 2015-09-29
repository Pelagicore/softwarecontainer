add_definitions(-DLXCTEMPLATE="${LXC_TEMPLATE_DIR}/lxc-pelagicontain")

SET(LIBPELAGICONTAIN_DIR ${CMAKE_CURRENT_SOURCE_DIR})
add_subdirectory(include)
add_subdirectory(src)

if(ENABLE_TEST)
	add_subdirectory(unit-test)
endif(ENABLE_TEST)

configure_file(
	${PROJECT_SOURCE_DIR}/libpelagicontain/lxc-pelagicontain.in
	${CMAKE_BINARY_DIR}/libpelagicontain/lxc-pelagicontain
)
install(PROGRAMS ${CMAKE_BINARY_DIR}/libpelagicontain/lxc-pelagicontain DESTINATION ${LXC_TEMPLATE_DIR})
install(FILES pelagicontain.conf DESTINATION ${SYS_CONFIG_DIR})
