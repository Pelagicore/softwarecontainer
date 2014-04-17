# Written by Nathan Samson <nathansamson@gmail.com>, 2011
#
# License: Public domain.
#
# Defines
#
# DBUSCPPGLIB_INCLUDE_DIRS
# which contains the include directory for dbus-c++/dbus.h
#
# DBUSCPPGLIB_LIBRARIES
# which contains the library directory for libdbus-c++-1

#                        
find_path(DBUSCPPGLIB_INCLUDE_DIR dbus-c++/dbus.h
	PATH_SUFFIXES include/dbus-c++-1)

find_library(DBUSCPPGLIB_LIBRARIES dbus-c++-1
	PATH_SUFFIXES lib)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(DBusCpp DEFAULT_MSG
	DBUSCPPGLIB_LIBRARIES
	DBUSCPPGLIB_INCLUDE_DIR)

# - Try to find the DBus-c++-1 library
# Once done this will define
#
# DBUSCPPGLIB_FOUND - system has DBus-c++
# DBUSCPPGLIB_INCLUDE_DIR - the DBus-c++ include directory
# DBUSCPPGLIB_ARCH_INCLUDE_DIR - the DBus-cpp architecture-specific include directory
# DBUSCPPGLIB_LIBRARIES - the libraries needed to use DBus-cpp
#
# modeled after FindDbus.cmake:
# Copyright (c) 2008, Kevin Kofler, <kevin.kofler@chello.at>
# modeled after FindLibArt.cmake:
# Copyright (c) 2006, Alexander Neundorf, <neundorf@kde.org>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.
if (DBUSCPPGLIB_INCLUDE_DIR AND DBUSCPPGLIB_LIBRARIES)
	# in cache already
	SET(DBUSCPPGLIB_FOUND TRUE)
else (DBUSCPPGLIB_INCLUDE_DIR AND DBUSCPPGLIB_LIBRARIES)
	IF (NOT WIN32)
		FIND_PACKAGE(PkgConfig)
		IF (PKG_CONFIG_FOUND)
			# use pkg-config to get the directories and then use these values
			# in the FIND_PATH() and FIND_LIBRARY() calls
			pkg_check_modules(_DBUSCPPGLIB_PC QUIET dbus-c++-1)
		ENDIF (PKG_CONFIG_FOUND)
	ENDIF (NOT WIN32)
	FIND_PATH(DBUSCPPGLIB_INCLUDE_DIR dbus-c++/dbus.h
		${_DBUSCPPGLIB_PC_INCLUDE_DIRS}
		/usr/include
		/usr/include/dbus-c++-1
		/usr/local/include
		)
	FIND_LIBRARY(DBUSCPPGLIB_LIBRARIES NAMES dbus-c++-1
		PATHS
		${_DBUSCPPGLIB_PC_LIBDIR}
		)
	if (DBUSCPPGLIB_INCLUDE_DIR AND DBUSCPPGLIB_LIBRARIES)
		set(DBUS_FOUND TRUE)
	endif (DBUSCPPGLIB_INCLUDE_DIR AND DBUSCPPGLIB_LIBRARIES)
	if (DBUSCPPGLIB_FOUND)
		if (NOT DBusCpp_FIND_QUIETLY)
			message(STATUS "Found DBus-C++: ${DBUSCPPGLIB_LIBRARIES}, ${DBUSCPPGLIB_INCLUDE_DIR}")
		endif (NOT DBusCpp_FIND_QUIETLY)
	else (DBUSCPPGLIB_FOUND)
		if (DBusCpp_FIND_REQUIRED)
			message(FATAL_ERROR "Could NOT find DBus-C++")
		endif (DBusCpp_FIND_REQUIRED)
	endif (DBUSCPPGLIB_FOUND)
	MARK_AS_ADVANCED(DBUSCPPGLIB_INCLUDE_DIR DBUSCPPGLIB_LIBRARIES)
endif (DBUSCPPGLIB_INCLUDE_DIR AND DBUSCPPGLIB_LIBRARIES)
