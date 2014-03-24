pkg_check_modules(DEPENDENCIES REQUIRED libpulse jansson)
add_definitions(${DEPENDENCIES_CFLAGS})

link_directories(
	${DEPENDENCIES_LIBRARY_DIRS} ${PELAGICORE_UTILS_LIBRARY_DIRS})

add_definitions(-DCONFIG="${SYS_CONFIG_DIR}/pelagicontain.conf")


find_program(DBUSBINDINGTOOL "dbusxx-xml2cpp")
macro(generate_dbus_hfile type xml h)
    add_custom_command(
        OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/${h}
        COMMAND dbusxx-xml2cpp ../../shared-service-interfaces/${xml} --${type}=${h}
        DEPENDS ../../shared-service-interfaces/${xml}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )
endmacro(generate_dbus_hfile type xml h)

if(DBUSBINDINGTOOL)
    generate_dbus_hfile("proxy" "pam-service.xml" "pamproxy.h")
    generate_dbus_hfile("adaptor" "pelagicontain-service.xml" "dbusadaptor.h")
else(DBUSBINDINGTOOL)
    MESSAGE(ERROR " Cannot find dbusxx-xml2cpp")
endif(DBUSBINDINGTOOL)


add_executable( pelagicontain
    configparser.cpp
    container.cpp
    controllerinterface.cpp
    dbusadaptor.h
    dbusgateway.cpp
    dbusmainloop.cpp
    devicenodegateway.cpp
    generators.cpp
    main.cpp
    networkgateway.cpp
    paminterface.cpp
    pamproxy.h
    pelagicontain.cpp
    pelagicontaintodbusadapter.cpp
    pulsegateway.cpp
)

TARGET_LINK_LIBRARIES( pelagicontain
    ${PELAGICORE_UTILS_LIBRARIES}
    ${DEPENDENCIES_LIBRARIES}
    ${DBUS_LIBRARIES} ${DBUSCPP_LIBRARIES}
)

install(TARGETS pelagicontain RUNTIME DESTINATION bin)
