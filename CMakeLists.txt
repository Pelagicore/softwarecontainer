cmake_minimum_required(VERSION 2.6)

project(pelagicontain)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake_modules)

SET( ${PROJECT_NAME}_MAJOR_VERSION 0 )
SET( ${PROJECT_NAME}_MINOR_VERSION 9 )
SET( ${PROJECT_NAME}_PATCH_LEVEL 1 )

set(VERSION ${${PROJECT_NAME}_MAJOR_VERSION}.${${PROJECT_NAME}_MINOR_VERSION}.${${PROJECT_NAME}_PATCH_LEVEL})

if(EXTRA_PKG_CONFIG_PATH)
	set(ENV{PKG_CONFIG_PATH} ${EXTRA_PKG_CONFIG_PATH})
	message("Using PKG_CONFIG_PATH: " ${EXTRA_PKG_CONFIG_PATH})
endif(EXTRA_PKG_CONFIG_PATH)

add_definitions(-DPACKAGE_VERSION="0.1")

find_package(PelagicoreUtilities REQUIRED)
find_package(IVILogging          REQUIRED)

pkg_config_use_package(DBUSCPP dbus-c++-glib-1)
pkg_config_use_package(GLIBMM glibmm-2.4)
pkg_config_use_package(LXC lxc)

if(ADDITIONAL_LD_LIBRARY_PATH)
    add_definitions(-DADDITIONAL_LD_LIBRARY_PATH=${ADDITIONAL_LD_LIBRARY_PATH})
endif()

if(ADDITIONAL_PATH)
    add_definitions(-DADDITIONAL_PATH=${ADDITIONAL_PATH})
endif()

if(${CMAKE_INSTALL_PREFIX} STREQUAL "/usr")
	SET(SYS_CONFIG_DIR /etc)
	SET(LXC_TEMPLATE_DIR /var/hl/lxc-templates/)
	SET(LXC_HOOK_SCRIPTS_DIR /var/hl/lxc-hook-scripts/)
else()
	SET(SYS_CONFIG_DIR ${CMAKE_INSTALL_PREFIX}/etc)
	SET(LXC_TEMPLATE_DIR ${CMAKE_INSTALL_PREFIX}/var/hl/lxc-templates)
	SET(LXC_HOOK_SCRIPTS_DIR ${CMAKE_INSTALL_PREFIX}/var/hl/lxc-hook-scripts)
endif()
message("Configuration files will be installed in " ${SYS_CONFIG_DIR})
message("LXC template will be installed in " ${LXC_TEMPLATE_DIR})
message("LXC hook scripts will be installed in " ${LXC_HOOK_SCRIPTS_DIR})

include_directories(
	common
	clientLib
	pelagicontain/src
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_BINARY_DIR}/pelagicontain/src
)

option(ENABLE_TEST "Enables unit testing" ON)
if(ENABLE_TEST)
	enable_testing()
	add_gmock()
endif()

option(ENABLE_SYSTEMD "Enables installation of systemD unit files" ON)

add_coverage()

install(FILES scripts/setup_pelagicontain.sh DESTINATION bin PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)

add_definitions(-DINSTALL_PREFIX="${CMAKE_INSTALL_PREFIX}")



configure_file(pelagicontain.pc.in pelagicontain.pc @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/pelagicontain.pc DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/pkgconfig)

set(DEVELOPMENT_INCLUDE_PATH " -I${CMAKE_CURRENT_SOURCE_DIR}/agent/lib -I${CMAKE_CURRENT_SOURCE_DIR}/clientLib -I${CMAKE_CURRENT_BINARY_DIR} -I${CMAKE_CURRENT_SOURCE_DIR}/pelagicontain/src -I${CMAKE_CURRENT_SOURCE_DIR}/common #")
set(DEVELOPMENT_LIBRARY_PATH " -L${CMAKE_CURRENT_BINARY_DIR}/clientLib -L${CMAKE_CURRENT_BINARY_DIR}/lib #" )
configure_file(pelagicontain.pc.in pelagicontain-uninstalled.pc @ONLY)
if(INSTALL_PKGCONFIG_UNINSTALLED_FILE)
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/pelagicontain-uninstalled.pc DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/pkgconfig)
endif()

configure_file(pelagicontain-config.h.in pelagicontain-config.h @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/pelagicontain-config.h DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/pelagicontain)

add_subdirectory(pelagicontain)
add_subdirectory(clientLib)

add_subdirectory(agent)

add_subdirectory(doc)
