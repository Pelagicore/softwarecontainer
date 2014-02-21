cmake_minimum_required(VERSION 2.6)

project(pelagicontain)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake_modules)

if(EXTRA_PKG_CONFIG_PATH)
	set(ENV{PKG_CONFIG_PATH} ${EXTRA_PKG_CONFIG_PATH})
	message("Using PKG_CONFIG_PATH: " ${EXTRA_PKG_CONFIG_PATH})
endif(EXTRA_PKG_CONFIG_PATH)

find_package(PkgConfig           REQUIRED)
find_package(PelagicoreUtilities REQUIRED)
find_package(DBusCpp             REQUIRED)

add_definitions(-DPACKAGE_VERSION="0.1")

if(${CMAKE_INSTALL_PREFIX} STREQUAL "/usr")
	SET(SYS_CONFIG_DIR /etc)
	SET(LXC_TEMPLATE_DIR /var/hl/lxc-templates/)
else()
	SET(SYS_CONFIG_DIR ${CMAKE_INSTALL_PREFIX}/etc)
	SET(LXC_TEMPLATE_DIR ${CMAKE_INSTALL_PREFIX}/var/hl/lxc-templates)
endif()
message("Configuration files will be installed in " ${SYS_CONFIG_DIR})
message("LXC template will be installed in " ${LXC_TEMPLATE_DIR})

include_directories (${DBUS_INCLUDE_DIR} ${DBUSCPP_INCLUDE_DIR})

add_subdirectory(pelagicontain)
add_subdirectory(controller)

option(BUILD_TESTS "Enables unit testing" OFF)
if(BUILD_TESTS)
	enable_testing()
endif(BUILD_TESTS)
