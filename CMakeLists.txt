
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


cmake_minimum_required(VERSION 3.0.2)

project(softwarecontainer)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake_modules)

# load GNUInstallDirs module
include(GNUInstallDirs)

#
# Set software version
#
set(${PROJECT_NAME}_MAJOR_VERSION 0)
set(${PROJECT_NAME}_MINOR_VERSION 9)
set(${PROJECT_NAME}_PATCH_LEVEL 1)
set(VERSION ${${PROJECT_NAME}_MAJOR_VERSION}.${${PROJECT_NAME}_MINOR_VERSION}.${${PROJECT_NAME}_PATCH_LEVEL})
add_definitions(-DPACKAGE_VERSION="${VERSION}")

execute_process(COMMAND "/usr/bin/git" "rev-parse" "--short" "HEAD" OUTPUT_VARIABLE REVISION_RAW)

string(REPLACE "\n" "" REVISION "${REVISION_RAW}")
string(TIMESTAMP DATE "%Y-%m-%d")

#
# Find dependencies, and include them etc
#

if(EXTRA_PKG_CONFIG_PATH)
    set(ENV{PKG_CONFIG_PATH} ${EXTRA_PKG_CONFIG_PATH})
    message(STATUS "Using PKG_CONFIG_PATH: " ${EXTRA_PKG_CONFIG_PATH})
endif(EXTRA_PKG_CONFIG_PATH)

find_package(PkgConfig          REQUIRED)

pkg_check_modules(IVILogging    REQUIRED ivi-logging>=1.3.0)
pkg_check_modules(DBusCpp       REQUIRED dbus-c++-glib-1>=0.9.0)
pkg_check_modules(Glibmm        REQUIRED glibmm-2.4>=2.42.0)
pkg_check_modules(LXC           REQUIRED lxc>=2.0.0)
pkg_check_modules(Jansson       REQUIRED jansson>=2.6)

add_definitions(${IVILogging_CFLAGS_OTHER})
add_definitions(${DBusCpp_CFLAGS_OTHER})
add_definitions(${Glibmm_CFLAGS_OTHER})
add_definitions(${LXC_CFLAGS_OTHER})
add_definitions(${Jansson_CFLAGS_OTHER})
add_definitions(-DINSTALL_BINDIR="${CMAKE_INSTALL_FULL_BINDIR}")

include_directories(${IVILogging_INCLUDE_DIRS})
include_directories(${DBusCpp_INCLUDE_DIRS})
include_directories(${Glibmm_INCLUDE_DIRS})
include_directories(${LXC_INCLUDE_DIRS})
include_directories(${Jansson_INCLUDE_DIRS})

include_directories(
    common
    libsoftwarecontainer/src
    libsoftwarecontainer/include
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}/libsoftwarecontainer/src
)

link_directories(${IVILogging_LIBRARY_DIRS})
link_directories(${DBusCpp_LIBRARY_DIRS})
link_directories(${Glibmm_LIBRARY_DIRS})
link_directories(${LXC_LIBRARY_DIRS})
link_directories(${Jansson_LIBRARY_DIRS})

#
# All available options goes here
#

# Options on enabling / disabling gateways at compile-time
option(ENABLE_PULSEGATEWAY "Enables pulse gateway" ON)
option(ENABLE_NETWORKGATEWAY "Enables network gateway" ON)
option(ENABLE_DEVICENODEGATEWAY "Enables device node gateway" ON)
option(ENABLE_DBUSGATEWAY "Enables DBus gateway" ON)
option(ENABLE_CGROUPSGATEWAY "Enables cgroups gateway" ON)

# Options on various things
option(ENABLE_SYSTEMD "Installs systemd service files" ON)
option(ENABLE_TEST "Enables unit testing" ON)
option(ENABLE_PROFILING "Enables profiling support in the application" OFF)
option(ENABLE_COVERAGE "Enable coverage support in the build" OFF)
option(ENABLE_EXAMPLES "Enable building and installation of examples" OFF)
option(CREATE_BRIDGE "SoftwareContainer will create network bridge if missing" ON)

# Options for documentation
option(ENABLE_USER_DOC "Enable building and installation of the user documentation" OFF)
option(ENABLE_API_DOC "Enable building and installation of the api documentation" OFF)
option(ENABLE_ALL_DOC "Enable building and installation of both the user and the api documentation." OFF)

if(ENABLE_ALL_DOC)
    set(ENABLE_API_DOC ON)
    set(ENABLE_USER_DOC ON)
endif()

#
# RPATH options
#

# use, i.e. don't skip the full RPATH for the build tree
set(CMAKE_SKIP_BUILD_RPATH  FALSE)

# when building, don't use the install RPATH already (but later on when installing)
set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_FULL_LIBDIR}")

# add the automatically determined parts of the RPATH
# which point to directories outside the build tree to the install RPATH
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

# the RPATH to be used when installing, but only if it's not a system directory
list(FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES "${CMAKE_INSTALL_FULL_LIBDIR}" isSystemDir)
if("${isSystemDir}" STREQUAL "-1")
   set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_FULL_LIBDIR}")
endif("${isSystemDir}" STREQUAL "-1")

SET(SYS_CONFIG_DIR ${CMAKE_INSTALL_FULL_SYSCONFDIR})
SET(LXC_TEMPLATE_DIR ${CMAKE_INSTALL_FULL_DATAROOTDIR}/softwarecontainer/lxc-templates)
message(STATUS "Configuration files will be installed in " ${SYS_CONFIG_DIR})
message(STATUS "LXC template will be installed in " ${LXC_TEMPLATE_DIR})

#
# Set C++ compiler flags
#

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wunused -Wall -std=c++11 -pedantic -Wextra")
if(ENABLE_PROFILING)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DPROFILING_ENABLED=1")
endif()

if(ENABLE_TEST)
    include(AddGMock)
    add_gmock()
    configure_file(scripts/run-tests.sh run-tests.sh COPYONLY)
endif()

if(ENABLE_COVERAGE)
    include(AddLCov)
    add_coverage()
endif(ENABLE_COVERAGE)

#
# Network related options
#

if (ENABLE_NETWORKGATEWAY)

    macro(set_default VAR VALUE)
        if(NOT DEFINED ${VAR})
            set(${VAR} ${VALUE})
        endif()
    endmacro()

    set_default(BRIDGE_DEVICE lxcbr0)
    add_definitions(-DBRIDGE_DEVICE="${BRIDGE_DEVICE}")
    message(STATUS "Network bridge defined as ${BRIDGE_DEVICE}")

    #
    # Some settings for whether or not SC should create a bridge
    # if one is not found.
    #
    if(CREATE_BRIDGE)
        message(STATUS "SoftwareContainer will create ${BRIDGE_DEVICE} if not present at runtime")
        add_definitions(-DCREATE_BRIDGE=true)

        set_default(BRIDGE_IP 10.0.3.1)
        set_default(BRIDGE_NETADDR 10.0.3.0)
        set_default(BRIDGE_NETADDR 10.0.3.0)
        set_default(BRIDGE_NETMASK 255.255.255.0)
        set_default(BRIDGE_NETMASK_BITS 24)

        configure_file(scripts/setup_create_nobridge.sh.in ${CMAKE_CURRENT_BINARY_DIR}/create_bridge.sh)
        file(READ ${CMAKE_CURRENT_BINARY_DIR}/create_bridge.sh NOBRIDGE)

    else()
        message(WARNING "SoftwareContainer will exit if ${BRIDGE_DEVICE} is not present at runtime")
        configure_file(scripts/setup_exit_nobridge.sh.in ${CMAKE_CURRENT_BINARY_DIR}/exit_nobridge.sh)
        file(READ exit_nobridge.sh NOBRIDGE)
    endif()

    # Get the actual setup script
    configure_file(scripts/setup_softwarecontainer.sh.in ${CMAKE_CURRENT_BINARY_DIR}/setup_softwarecontainer.sh)
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/setup_softwarecontainer.sh DESTINATION bin
            PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
endif()

#
# Pkg-config file
#
configure_file(softwarecontainer.pc.in softwarecontainer.pc @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/softwarecontainer.pc DESTINATION ${CMAKE_INSTALL_FULL_LIBDIR}/pkgconfig)

#
# General configuration file
#
configure_file(softwarecontainer-config.h.in softwarecontainer-config.h @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/softwarecontainer-config.h DESTINATION ${CMAKE_INSTALL_FULL_LIBDIR}/softwarecontainer)

#
# Add all sub-projects
#

add_subdirectory(common)
add_subdirectory(libsoftwarecontainer)

add_subdirectory(agent)
add_subdirectory(libsoftwarecontaineragent)

if (ENABLE_EXAMPLES)
    add_subdirectory(examples)
endif()

add_subdirectory(doc)
