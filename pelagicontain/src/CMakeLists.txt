# Default values
option(ENABLE_PULSEGATEWAY "Enables pulse gateway" ON)
option(ENABLE_NETWORKGATEWAY "Enables network gateway" ON)
option(ENABLE_DEVICENODEGATEWAY "Enables device node gateway" ON)
option(ENABLE_DBUSGATEWAY "Enables DBus gateway" ON)
option(ENABLE_CGROUPSGATEWAY "Enables cgroups gateway" ON)

# Pulse is a special case since it has a dependency package
if(ENABLE_PULSEGATEWAY)
    pkg_check_modules(DEPENDENCIES REQUIRED libpulse)
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

add_library(pelagicontainLib SHARED 
    container.cpp
    generators.cpp
    gateway.cpp
    pelagicontain.cpp
    pelagicontain-lib.cpp
    filegateway.cpp
    waylandgateway.cpp
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
    ${COMMON_DEPENDENCIES_LIBRARIES}
)

set_target_properties(pelagicontainLib PROPERTIES VERSION ${VERSION} SOVERSION ${${PROJECT_NAME}_MAJOR_VERSION})

install(TARGETS pelagicontainLib DESTINATION lib)
