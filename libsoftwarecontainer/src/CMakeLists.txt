
# Copyright (C) 2016 Pelagicore AB
#
# Permission to use, copy, modify, and/or distribute this software for
# any purpose with or without fee is hereby granted, provided that the
# above copyright notice and this permission notice appear in all copies.
#
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
# WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR
# BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES
# OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
# WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
# ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
# SOFTWARE.
#
# For further information see LICENSE


# Enable or disable support for gateways compile-time.
SET(GATEWAY_SOURCES "")
macro(gateway_support flag strname filename)
    if (${flag})
        message(STATUS "Compiling with gateway support for ${strname}")
        list(APPEND GATEWAY_SOURCES gateway/${filename})
        add_definitions(-D${flag})
    else(${flag})
        message(STATUS "Compiling without gateway support for ${strname}")
    endif(${flag})
endmacro(gateway_support flag filename)

gateway_support(ENABLE_CGROUPSGATEWAY cgroups cgroups/cgroupsgateway.cpp)
gateway_support(ENABLE_DBUSGATEWAY DBus dbusgateway.cpp)
gateway_support(ENABLE_DEVICENODEGATEWAY DeviceNode devicenodegateway.cpp)
gateway_support(ENABLE_WAYLANDGATEWAY Wayland waylandgateway.cpp)
gateway_support(ENABLE_NETWORKGATEWAY network network/networkgateway.cpp)
if(ENABLE_NETWORKGATEWAY)
    list(APPEND GATEWAY_SOURCES netlink.cpp)
    list(APPEND GATEWAY_SOURCES gateway/network/iptableentry.cpp)
    list(APPEND GATEWAY_SOURCES gateway/network/networkgatewayparser.cpp)
endif()

gateway_support(ENABLE_PULSEGATEWAY pulse pulsegateway.cpp)
gateway_support(ENABLE_ENVGATEWAY environment envgateway.cpp)
gateway_support(ENABLE_FILEGATEWAY files filegateway.cpp)

add_library(softwarecontainerlib SHARED
    container.cpp
    gateway/gateway.cpp
    softwarecontainer.cpp
    workspace.cpp
    jobs/jobabstract.cpp
    jobs/functionjob.cpp
    jobs/commandjob.cpp
    ${GATEWAY_SOURCES}
)

TARGET_LINK_LIBRARIES( softwarecontainerlib
    softwarecontainercommon
    ${DBusCpp_LIBRARIES}
    ${Glibmm_LIBRARIES}
    ${LXC_LIBRARIES}
    ${Jansson_LIBRARIES}
)

include_directories(${LIBSOFTWARECONTAINER_DIR}/include)

install(TARGETS softwarecontainerlib DESTINATION lib)
set_target_properties(softwarecontainerlib PROPERTIES VERSION ${VERSION} SOVERSION ${${PROJECT_NAME}_MAJOR_VERSION})
