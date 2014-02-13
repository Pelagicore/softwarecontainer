pkg_check_modules(DEPENDENCIES REQUIRED libpulse jansson)
add_definitions(${DEPENDENCIES_CFLAGS})

link_directories(
	${DEPENDENCIES_LIBRARY_DIRS} ${PELAGICORE_UTILS_LIBRARY_DIRS})

add_definitions(-DCONFIG="${SYS_CONFIG_DIR}/pelagicontain.conf")

add_executable( pelagicontain
	pelagicontaintodbusadapter.cpp
	dbusmainloop.cpp
	main.cpp
	configparser.cpp
	container.cpp
	dbusgateway.cpp
	generators.cpp
	pelagicontain.cpp
	pulsegateway.cpp
	controllerinterface.cpp
	paminterface.cpp
	networkgateway.cpp
)

TARGET_LINK_LIBRARIES( pelagicontain
	${PELAGICORE_UTILS_LIBRARIES}
	${DEPENDENCIES_LIBRARIES}
	${DBUS_LIBRARIES} ${DBUSCPP_LIBRARIES}
)

install(TARGETS pelagicontain RUNTIME DESTINATION bin)

install(FILES lxc-pelagicontain DESTINATION share/lxc/templates)
install(FILES pelagicontain.conf DESTINATION ${SYS_CONFIG_DIR})
