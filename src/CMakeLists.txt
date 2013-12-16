pkg_check_modules(DEPENDENCIES REQUIRED libpulse jansson)
add_definitions(${DEPENDENCIES_CFLAGS})

link_directories(
	${DEPENDENCIES_DIRS}
)

add_executable( pelagicontain
	config.cpp
	container.cpp
	dbusproxy.cpp
	generators.cpp
	iptables.cpp
	pelagicontain.cpp
	pulse.cpp
	trafficcontrol.cpp
)

TARGET_LINK_LIBRARIES( pelagicontain
	${PELAGICORE_UTILS_LIBRARIES}
	${DEPENDENCIES_LIBRARIES}
)

install(TARGETS pelagicontain RUNTIME DESTINATION bin)

install(FILES lxc-pelagicontain DESTINATION share/lxc/templates/lxc-pelagicontain)
install(FILES pela_lxc.conf DESTINATION ${SYS_CONFIG_DIR}/pelagicontain)
