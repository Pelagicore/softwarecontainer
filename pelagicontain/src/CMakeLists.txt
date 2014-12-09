# Default values
option(ENABLE_PULSEGATEWAY "Enables pulse gateway" ON)
option(ENABLE_NETWORKGATEWAY "Enables network gateway" ON)
option(ENABLE_DEVICENODEGATEWAY "Enables device node gateway" ON)
option(ENABLE_DBUSGATEWAY "Enables DBus gateway" ON)
option(ENABLE_CGROUPSGATEWAY "Enables cgroups gateway" ON)

# Pulse is a special case since it has a dependency package
if(ENABLE_PULSEGATEWAY)
    pkg_check_modules(DEPENDENCIES REQUIRED libpulse jansson)
else(ENABLE_PULSEGATEWAY)
    pkg_check_modules(DEPENDENCIES REQUIRED jansson)
endif(ENABLE_PULSEGATEWAY)
add_definitions(${DEPENDENCIES_CFLAGS})

# Enable or disable support for gateways compile-time.
SET(GATEWAY_SOURCES "")
macro(gateway_support flag strname filename)
    if (${flag})
        MESSAGE(INFO " Compiling with gateway support for ${strname}")
        LIST(APPEND GATEWAY_SOURCES ${filename})
        ADD_DEFINITIONS(-D${flag})
    else(${flag})
        MESSAGE(INFO " Compiling without gateway support for ${strname}")
    endif(${flag})
endmacro(gateway_support flag filename)

gateway_support(ENABLE_PULSEGATEWAY pulse pulsegateway.cpp)
gateway_support(ENABLE_NETWORKGATEWAY network networkgateway.cpp)
gateway_support(ENABLE_DBUSGATEWAY DBus dbusgateway.cpp)
gateway_support(ENABLE_DEVICENODEGATEWAY DeviceNode devicenodegateway.cpp)
gateway_support(ENABLE_CGROUPSGATEWAY cgroups cgroupsgateway.cpp)

find_package(SharedServiceInterfaces REQUIRED)

link_directories(
    ${DEPENDENCIES_LIBRARY_DIRS} 
    ${PELAGICORE_UTILS_LIBRARY_DIRS} 
    ${GLIBMM_LIBRARY_DIRS}
)



find_program(DBUSBINDINGTOOL "dbusxx-xml2cpp")
macro(generate_dbus_hfile type xml h)
    add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${h}
        COMMAND dbusxx-xml2cpp ${SHARED_SERVICE_INTERFACES_PATH}/${xml} --${type}=${h}
        DEPENDS ${SHARED_SERVICE_INTERFACES_PATH}/${xml}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )
endmacro(generate_dbus_hfile type xml h)

if(DBUSBINDINGTOOL)
    generate_dbus_hfile("adaptor" "pelagicontain-service.xml" "dbusadaptor.h")
else(DBUSBINDINGTOOL)
    MESSAGE(ERROR " Cannot find dbusxx-xml2cpp")
endif(DBUSBINDINGTOOL)

add_library(pelagicontainLib SHARED 
    pelagicontaintodbusadapter.cpp
    dbusmainloop.cpp
    configparser.cpp
    container.cpp
    generators.cpp
    gateway.cpp
    pelagicontain.cpp
    dbusadaptor.h
    systemcallinterface.cpp
    pelagicontain-lib.cpp
#    dltgateway.cpp
    filegateway.cpp
    waylandgateway.cpp
    ${PROJECT_SOURCE_DIR}/common/config.cpp
    ${PROJECT_SOURCE_DIR}/common/common.cpp
    ${GATEWAY_SOURCES}
)

TARGET_LINK_LIBRARIES( pelagicontainLib
    ${PELAGICORE_UTILS_LIBRARIES}
    ${DEPENDENCIES_LIBRARIES}
    ${DBUS_LIBRARIES} ${DBUSCPP_LIBRARIES}
    ${DBUSCPPGLIB_LIBRARIES}
    ${GLIBMM_LIBRARIES}
    ${LXC_LIBRARIES}
)

set_target_properties(pelagicontainLib PROPERTIES VERSION ${VERSION} SOVERSION ${${PROJECT_NAME}_MAJOR_VERSION})

install(TARGETS pelagicontainLib DESTINATION lib)

add_executable( pelagicontain
    main.cpp
)

TARGET_LINK_LIBRARIES( pelagicontain
    pelagicontainLib
)

install(TARGETS pelagicontain RUNTIME DESTINATION bin)
