cmake_minimum_required(VERSION 2.6)

PROJECT(pelagicontain)

FIND_PACKAGE(PkgConfig REQUIRED)
FIND_PACKAGE(PelagicoreUtilities REQUIRED)

add_definitions(-DPACKAGE_VERSION="0.1")

IF(${CMAKE_INSTALL_PREFIX} STREQUAL "/usr")
	SET(SYS_CONFIG_DIR /etc)
ELSE()
	SET(SYS_CONFIG_DIR ${CMAKE_INSTALL_PREFIX}/etc)
ENDIF()
MESSAGE("Configuration files will be installed in " ${SYS_CONFIG_DIR})

add_subdirectory(src)
