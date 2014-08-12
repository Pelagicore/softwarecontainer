# Default values
option(ENABLE_PULSEGW "Enables pulse gateway" ON)
option(ENABLE_NETGW "Enables network gateway" ON)
option(ENABLE_DEVICENODEGW "Enables device node gateway" ON)
option(ENABLE_DBUSGW "Enables DBus gateway" ON)

# Pulse is a special case since it has a dependency package
if(ENABLE_PULSEGW)
    pkg_check_modules(DEPENDENCIES REQUIRED libpulse jansson)
else(ENABLE_PULSEGW)
    pkg_check_modules(DEPENDENCIES REQUIRED jansson)
endif(ENABLE_PULSEGW)
add_definitions(${DEPENDENCIES_CFLAGS})

# Enable or disable support for gateways compile-time.
SET(GWSOURCES "")
macro(gateway_support flag strname filename)
    if (${flag})
        MESSAGE(INFO " Compiling with gateway support for ${strname}")
        LIST(APPEND GWSOURCES ${filename})
        ADD_DEFINITIONS(-D${flag})
    else(${flag})
        MESSAGE(INFO " Compiling without gateway support for ${strname}")
    endif(${flag})
endmacro(gateway_support flag filename)

gateway_support(ENABLE_PULSEGW pulse pulsegateway.cpp)
gateway_support(ENABLE_NETGW network networkgateway.cpp)
gateway_support(ENABLE_DBUSGW DBus dbusgateway.cpp)
gateway_support(ENABLE_DEVICENODEGW DeviceNode devicenodegateway.cpp)

find_package(SharedServiceInterfaces REQUIRED)

link_directories(
    ${DEPENDENCIES_LIBRARY_DIRS} 
    ${PELAGICORE_UTILS_LIBRARY_DIRS} 
    ${GLIBMM_LIBRARY_DIRS}
)

add_definitions(-DCONFIG="${SYS_CONFIG_DIR}/pelagicontain.conf")


find_program(DBUSBINDINGTOOL "dbusxx-xml2cpp")
macro(generate_dbus_hfile type xml h)
    add_custom_command(
        OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/${h}
        COMMAND dbusxx-xml2cpp ${SHARED_SERVICE_INTERFACES_PATH}/${xml} --${type}=${h}
        DEPENDS ${SHARED_SERVICE_INTERFACES_PATH}/${xml}
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
    pelagicontaintodbusadapter.cpp
    dbusmainloop.cpp
    main.cpp
    configparser.cpp
    container.cpp
    generators.cpp
    pelagicontain.cpp
    controllerinterface.cpp
    paminterface.cpp
    pamproxy.h
    dbusadaptor.h
    systemcallinterface.cpp
    ${PROJECT_SOURCE_DIR}/common/config.cpp
    ${GWSOURCES}
)

TARGET_LINK_LIBRARIES( pelagicontain
    ${PELAGICORE_UTILS_LIBRARIES}
    ${DEPENDENCIES_LIBRARIES}
    ${DBUS_LIBRARIES} ${DBUSCPP_LIBRARIES}
    ${DBUSCPPGLIB_LIBRARIES}
    ${GLIBMM_LIBRARIES}
)

install(TARGETS pelagicontain RUNTIME DESTINATION bin)
