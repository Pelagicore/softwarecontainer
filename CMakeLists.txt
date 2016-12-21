
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

# load the GNUInstallDirs module to get some standard paths defined, and load the FeatureSummary
# module to be able to print a nice summary of configuration in the end of the script.
include(GNUInstallDirs)
include(FeatureSummary)

#
# Set software version
#
set(${PROJECT_NAME}_MAJOR_VERSION 0)
set(${PROJECT_NAME}_MINOR_VERSION 14)
set(${PROJECT_NAME}_PATCH_LEVEL 0)
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
endif(EXTRA_PKG_CONFIG_PATH)

find_package(PkgConfig          REQUIRED)

pkg_check_modules(IVILogging    REQUIRED ivi-logging>=1.3.0)
pkg_check_modules(DBusCpp       REQUIRED dbus-c++-glib-1>=0.9.0)
pkg_check_modules(Glibmm        REQUIRED glibmm-2.4>=2.42.0)
pkg_check_modules(LXC           REQUIRED lxc>=2.0.0)
pkg_check_modules(Jansson       REQUIRED jansson>=2.6)

# These are needed by all sub-projects
add_definitions(${IVILogging_CFLAGS_OTHER})
add_definitions(${Jansson_CFLAGS_OTHER})

add_definitions(${DBusCpp_CFLAGS_OTHER})
add_definitions(${Glibmm_CFLAGS_OTHER})
add_definitions(${LXC_CFLAGS_OTHER})
add_definitions(-DINSTALL_BINDIR="${CMAKE_INSTALL_FULL_BINDIR}")

include_directories(${IVILogging_INCLUDE_DIRS})
include_directories(${DBusCpp_INCLUDE_DIRS})
include_directories(${Glibmm_INCLUDE_DIRS})
include_directories(${LXC_INCLUDE_DIRS})
include_directories(${Jansson_INCLUDE_DIRS})

link_directories(${IVILogging_LIBRARY_DIRS})
link_directories(${DBusCpp_LIBRARY_DIRS})
link_directories(${Glibmm_LIBRARY_DIRS})
link_directories(${LXC_LIBRARY_DIRS})
link_directories(${Jansson_LIBRARY_DIRS})

include_directories(
    common
    libsoftwarecontainer/include
)

#
# All available options goes here
#

# Options on enabling / disabling gateways at compile-time
option(ENABLE_PULSEGATEWAY "Enables the pulse gateway" ON)
add_feature_info("PulseAudio Gateway" ENABLE_PULSEGATEWAY "Provides access to PulseAudio sockets")

option(ENABLE_NETWORKGATEWAY "Enables the network gateway" ON)
add_feature_info("Network Gateway" ENABLE_NETWORKGATEWAY "Provides firewalled network access")

option(ENABLE_DEVICENODEGATEWAY "Enables the device node gateway" ON)
add_feature_info("Device Node Gateway" ENABLE_DEVICENODEGATEWAY "Provides access to device nodes")

option(ENABLE_DBUSGATEWAY "Enables the DBus gateway" ON)
add_feature_info("D-Bus Gateway" ENABLE_DBUSGATEWAY "Allows access to D-Bus services")

option(ENABLE_CGROUPSGATEWAY "Enables the cgroups gateway" ON)
add_feature_info("CGroups Gateway" ENABLE_CGROUPSGATEWAY
                 "The CGroups gateway allows to set cgroups options for the container")

option(ENABLE_WAYLANDGATEWAY "Enables the wayland gateway" ON)
add_feature_info("Wayland Gateway" ENABLE_WAYLANDGATEWAY "Makes it possible to use wayland")

option(ENABLE_FILEGATEWAY "Enables the file gateway" ON)
add_feature_info("File Gateway" ENABLE_FILEGATEWAY
                 "Makes it possible to mount files and directories into the container")

option(ENABLE_ENVGATEWAY "Enables the environment gateway" ON)
add_feature_info("Environment Gateway" ENABLE_ENVGATEWAY
                 "Allows for setting environment variables in the container")

# Options on various things
option(ENABLE_SYSTEMD "Installs systemd service files" ON)
add_feature_info("SystemD Service File" ENABLE_SYSTEMD "Install the systemd service files")

option(ENABLE_TEST "Enables unit testing" ON)
add_feature_info("Unit Tests" ENABLE_TEST "Build the unit tests")

option(ENABLE_PROFILING "Enables profiling support in the application" OFF)
add_feature_info("Profiling support" ENABLE_PROFILING "Enables profiling support")

option(ENABLE_COVERAGE "Enable coverage support in the build" OFF)
add_feature_info("Coverage support" ENABLE_COVERAGE "Build with coverage support")

option(ENABLE_EXAMPLES "Enable building and installation of examples" OFF)
add_feature_info("Examples" ENABLE_EXAMPLES "Enable building and installation of examples")

option(CREATE_BRIDGE "SoftwareContainer will create network bridge if missing" ON)
add_feature_info("Network Bridge" CREATE_BRIDGE "SoftwareContainer will create network bridge if missing")

option(ENABLE_SYSTEM_BUS "Enables the SoftwareContainerAgent to communicate over the system bus" ON)
add_feature_info("System D-Bus" ENABLE_SYSTEM_BUS "Install policy file for the D-Bus system bus")

# Options for documentation
option(ENABLE_USER_DOC "Enable building and installation of the user documentation" OFF)
add_feature_info("User documentation" ENABLE_USER_DOC "Enable building and installation of the user documentation")

option(ENABLE_API_DOC "Enable building and installation of the api documentation" OFF)
add_feature_info("API documentation" ENABLE_API_DOC "Enable building and installation of the api documentation")

option(ENABLE_ALL_DOC "Enable building and installation of both the user and the api documentation." OFF)
if(ENABLE_ALL_DOC)
    set(ENABLE_API_DOC ON)
    set(ENABLE_USER_DOC ON)
endif()

#
# Setup config default values, definitions, and configure files etc.
#
# These values are used to configure the general config file and are
# picked up by the ConfigDefaults class to be used for default config
# values.
#

SET(SYS_CONFIG_DIR ${CMAKE_INSTALL_FULL_SYSCONFDIR}/softwarecontainer/)
add_feature_info("Config directory" TRUE "Configuration files will be installed in ${SYS_CONFIG_DIR}")

# Since the general config file needs a lower case string as value
# for the booleans, we need to set specific variables for these
set(KEEP_CONTAINERS_ALIVE FALSE)
if(${KEEP_CONTAINERS_ALIVE})
    set(KEEP_CONTAINERS_ALIVE_VALUE_STRING "true")
else(${KEEP_CONTAINERS_ALIVE})
    set(KEEP_CONTAINERS_ALIVE_VALUE_STRING "false")
endif()

set(USE_SESSION_BUS FALSE)
if(${USE_SESSION_BUS})
    set(USE_SESSION_BUS_VALUE_STRING "true")
else(${USE_SESSION_BUS})
    set(USE_SESSION_BUS_VALUE_STRING "false")
endif()

set(PRELOAD_COUNT 0)
set(SHUTDOWN_TIMEOUT 1)
set(SHARED_MOUNTS_DIR "/tmp/container/")
set(LXC_CONFIG_PATH "${SYS_CONFIG_DIR}/softwarecontainer.conf")
set(SERVICE_MANIFEST_DIR "${SYS_CONFIG_DIR}/service-manifest.d/")
set(DEFAULT_SERVICE_MANIFEST_DIR "${SYS_CONFIG_DIR}/service-manifest.default.d/")

# Export definitions for the ConfigDefaults class
add_definitions(-DPRELOAD_COUNT=${PRELOAD_COUNT})
add_definitions(-DKEEP_CONTAINERS_ALIVE=${KEEP_CONTAINERS_ALIVE})
add_definitions(-DUSE_SESSION_BUS=${USE_SESSION_BUS})
add_definitions(-DSHUTDOWN_TIMEOUT=${SHUTDOWN_TIMEOUT})
add_definitions(-DSHARED_MOUNTS_DIR="${SHARED_MOUNTS_DIR}")
add_definitions(-DLXC_CONFIG_PATH="${LXC_CONFIG_PATH}")
add_definitions(-DSERVICE_MANIFEST_DIR="${SERVICE_MANIFEST_DIR}")
add_definitions(-DDEFAULT_SERVICE_MANIFEST_DIR="${DEFAULT_SERVICE_MANIFEST_DIR}")

# export_configs_needed_by_tests()
# NOTE: These definitons are only set because the "middle layer" tests currently need to create a Workspace
#       with valid values, see the setup code of SoftwareContainerTest test class.
add_definitions(-DSHARED_MOUNTS_DIR_TESTING="${SHARED_MOUNTS_DIR}")
add_definitions(-DLXC_CONFIG_PATH_TESTING="${LXC_CONFIG_PATH}")
add_definitions(-DSERVICE_MANIFEST_DIR_TESTING="${SERVICE_MANIFEST_DIR}")
add_definitions(-DDEFAULT_SERVICE_MANIFEST_DIR_TESTING="${DEFAULT_SERVICE_MANIFEST_DIR}")

# Set and define the path to the general config file, used by the agent
# as default location if nothing else is set on command line when starting
# the agent.
set(SC_CONFIG_FILE "${SYS_CONFIG_DIR}/softwarecontainer-config")
add_definitions(-DSC_CONFIG_FILE="${SC_CONFIG_FILE}")

SET(LXC_TEMPLATE_DIR ${CMAKE_INSTALL_FULL_DATAROOTDIR}/softwarecontainer/lxc-templates)
add_feature_info("LXC template" TRUE "LXC template will be installed in ${LXC_TEMPLATE_DIR}")

#
# RPATH options
#

# Use RPATH only if we're not installing to a system directory
list(FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES "${CMAKE_INSTALL_FULL_LIBDIR}" IS_SYSTEM_DIR)
if(${IS_SYSTEM_DIR} STREQUAL "-1")
    set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_FULL_LIBDIR}")
    set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
endif()

#
# Set C++ compiler flags
#

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wunused -Wall -std=c++11 -pedantic -Wextra")
if(ENABLE_PROFILING)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DPROFILING_ENABLED=1")
endif()

#
# Optionally include testing and coverage
#
if(ENABLE_TEST)
    include(AddGMock)
    add_gmock()
    configure_file(scripts/run-tests.py run-tests.py COPYONLY)
endif()

if(ENABLE_COVERAGE)
    include(AddLCov)
    add_coverage("${CMAKE_BINARY_DIR}/run-tests.py")
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
    add_feature_info("Network Bridge Device" TRUE "Network bridge defined as ${BRIDGE_DEVICE}")

    #
    # Some settings for whether or not SC should create a bridge
    # if one is not found.
    #
    if(CREATE_BRIDGE)
        add_definitions(-DCREATE_BRIDGE=true)

        set_default(BRIDGE_IP 10.0.3.1)
        set_default(BRIDGE_NETADDR 10.0.3.0)
        set_default(BRIDGE_NETADDR 10.0.3.0)
        set_default(BRIDGE_NETMASK 255.255.255.0)
        set_default(BRIDGE_NETMASK_BITS 24)

        configure_file(scripts/setup_create_nobridge.sh.in ${CMAKE_CURRENT_BINARY_DIR}/create_bridge.sh)
        file(READ ${CMAKE_CURRENT_BINARY_DIR}/create_bridge.sh NOBRIDGE)

    else()
        configure_file(scripts/setup_exit_nobridge.sh.in ${CMAKE_CURRENT_BINARY_DIR}/exit_nobridge.sh)
        file(READ exit_nobridge.sh NOBRIDGE)
    endif()

    # Get the actual setup script
    configure_file(scripts/setup_softwarecontainer.sh.in ${CMAKE_CURRENT_BINARY_DIR}/setup_softwarecontainer.sh)
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/setup_softwarecontainer.sh DESTINATION bin
            PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
endif()

#
# Add all sub-projects
#
add_subdirectory(common)
add_subdirectory(libsoftwarecontainer)

add_subdirectory(agent)
add_subdirectory(doc)

if (ENABLE_EXAMPLES)
    add_subdirectory(examples)
endif()

message("")
message("")
message(STATUS "SUMMARY OF CONFIGURATION")
message("")
feature_summary(WHAT DISABLED_FEATURES DESCRIPTION "Disabled features")
feature_summary(WHAT ENABLED_FEATURES DESCRIPTION "Enabled features and options")

