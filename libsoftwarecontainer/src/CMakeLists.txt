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
        LIST(APPEND GATEWAY_SOURCES gateway/${filename})
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

SET(DEFAULT_GATEWAYS TRUE)
gateway_support(DEFAULT_GATEWAYS env envgateway.cpp)
gateway_support(DEFAULT_GATEWAYS file filegateway.cpp)
gateway_support(DEFAULT_GATEWAYS wayland waylandgateway.cpp)

add_library(softwarecontainerLib SHARED 
    container.cpp
    generators.cpp
    libsoftwarecontainer.cpp
    gateway/gateway.cpp
    ${GATEWAY_SOURCES}
    ${PROJECT_SOURCE_DIR}/common/common.cpp
)

TARGET_LINK_LIBRARIES( softwarecontainerLib
    ${DEPENDENCIES_LIBRARIES}
    ${DBusCpp_LIBRARIES}
    ${Glibmm_LIBRARIES}
    ${LXC_LIBRARIES}
    ${IVIMainLoop_LIBRARIES}
    ${JSONParser_LIBRARIES}
)

include_directories(${LIBSOFTWARECONTAINER_DIR}/include)

install(TARGETS softwarecontainerLib DESTINATION lib)
set_target_properties(softwarecontainerLib PROPERTIES VERSION ${VERSION} SOVERSION ${${PROJECT_NAME}_MAJOR_VERSION})
