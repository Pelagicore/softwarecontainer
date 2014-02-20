cmake_minimum_required(VERSION 2.6)

add_subdirectory(src)
add_subdirectory(doc)

if(BUILD_TESTS)
	add_subdirectory(unit-test)
endif(BUILD_TESTS)

install(FILES lxc-pelagicontain DESTINATION share/lxc/templates)
install(FILES pelagicontain.conf DESTINATION ${SYS_CONFIG_DIR})
