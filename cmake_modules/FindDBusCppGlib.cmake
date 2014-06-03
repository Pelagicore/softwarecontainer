# Written by Nathan Samson <nathansamson@gmail.com>, 2011
#
# License: Public domain.
#
# Defines
#
# DBUSCPP_INCLUDE_DIRS
# which contains the include directory for dbus-c++/dbus.h
#
# DBUSCPP_LIBRARIES
# which contains the library directory for libdbus-c++-1

#                        
find_path(DBUSCPP_INCLUDE_DIR dbus-c++/dbus.h
	PATH_SUFFIXES include/dbus-c++-1)

find_library(DBUSCPP_LIBRARIES dbus-c++-glib-1
	PATH_SUFFIXES lib)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(DBusCpp DEFAULT_MSG
	DBUSCPP_LIBRARIES
	DBUSCPP_INCLUDE_DIR)

# - Try to find the DBus-c++-1 library
# Once done this will define
#
# DBUSCPP_FOUND - system has DBus-c++
# DBUSCPP_INCLUDE_DIR - the DBus-c++ include directory
# DBUSCPP_ARCH_INCLUDE_DIR - the DBus-cpp architecture-specific include directory
# DBUSCPP_LIBRARIES - the libraries needed to use DBus-cpp
#
# modeled after FindDbus.cmake:
# Copyright (c) 2008, Kevin Kofler, <kevin.kofler@chello.at>
# modeled after FindLibArt.cmake:
# Copyright (c) 2006, Alexander Neundorf, <neundorf@kde.org>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.
if (DBUSCPP_INCLUDE_DIR AND DBUSCPP_LIBRARIES)
	# in cache already
	SET(DBUSCPP_FOUND TRUE)
else (DBUSCPP_INCLUDE_DIR AND DBUSCPP_LIBRARIES)
	IF (NOT WIN32)
		FIND_PACKAGE(PkgConfig)
		IF (PKG_CONFIG_FOUND)
			# use pkg-config to get the directories and then use these values
			# in the FIND_PATH() and FIND_LIBRARY() calls
			pkg_check_modules(_DBUSCPP_PC QUIET dbus-c++-glib-1)
		ENDIF (PKG_CONFIG_FOUND)
	ENDIF (NOT WIN32)
	FIND_PATH(DBUSCPP_INCLUDE_DIR dbus-c++/dbus.h
		${_DBUSCPP_PC_INCLUDE_DIRS}
		/usr/include
		/usr/include/dbus-c++-1
		/usr/local/include
		)
	FIND_LIBRARY(DBUSCPP_LIBRARIES NAMES dbus-c++-glib-1
		PATHS
		${_DBUSCPP_PC_LIBDIR}
		)
	if (DBUSCPP_INCLUDE_DIR AND DBUSCPP_LIBRARIES)
		set(DBUS_FOUND TRUE)
	endif (DBUSCPP_INCLUDE_DIR AND DBUSCPP_LIBRARIES)
	if (DBUSCPP_FOUND)
		if (NOT DBusCpp_FIND_QUIETLY)
			message(STATUS "Found DBus-C++: ${DBUSCPP_LIBRARIES}, ${DBUSCPP_INCLUDE_DIR}")
		endif (NOT DBusCpp_FIND_QUIETLY)
	else (DBUSCPP_FOUND)
		if (DBusCpp_FIND_REQUIRED)
			message(FATAL_ERROR "Could NOT find DBus-C++")
		endif (DBusCpp_FIND_REQUIRED)
	endif (DBUSCPP_FOUND)
	MARK_AS_ADVANCED(DBUSCPP_INCLUDE_DIR DBUSCPP_LIBRARIES)
endif (DBUSCPP_INCLUDE_DIR AND DBUSCPP_LIBRARIES)
