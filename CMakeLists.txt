cmake_minimum_required(VERSION 2.6)

PROJECT(pelagicontain)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake_modules)

if(EXTRA_PKG_CONFIG_PATH)
	set(ENV{PKG_CONFIG_PATH} ${EXTRA_PKG_CONFIG_PATH})
	message("Using PKG_CONFIG_PATH: " ${EXTRA_PKG_CONFIG_PATH})
endif(EXTRA_PKG_CONFIG_PATH)

FIND_PACKAGE(PkgConfig           REQUIRED)
FIND_PACKAGE(PelagicoreUtilities REQUIRED)
find_package(DBusCpp             REQUIRED)

add_definitions(-DPACKAGE_VERSION="0.1")

IF(${CMAKE_INSTALL_PREFIX} STREQUAL "/usr")
	SET(SYS_CONFIG_DIR /etc)
ELSE()
	SET(SYS_CONFIG_DIR ${CMAKE_INSTALL_PREFIX}/etc)
ENDIF()
MESSAGE("Configuration files will be installed in " ${SYS_CONFIG_DIR})

INCLUDE_DIRECTORIES (${DBUS_INCLUDE_DIR} ${DBUSCPP_INCLUDE_DIR})

add_subdirectory(pelagicontain)
add_subdirectory(controller)

option(BUILD_TESTS "Enables unit testing" OFF)
if(BUILD_TESTS)
	enable_testing()
endif(BUILD_TESTS)
