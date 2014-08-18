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
find_package(DBusCppGlib         REQUIRED)

add_definitions(-DPACKAGE_VERSION="0.1")

pkg_check_modules(GLIBMM REQUIRED glibmm-2.4)
add_definitions(${GLIBMM_CFLAGS_OTHER})

pkg_check_modules(LXC REQUIRED lxc)
add_definitions(${LXC_CFLAGS_OTHER})

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
	${DBUS_INCLUDE_DIR}
	${DBUSCPP_INCLUDE_DIR}
	${DBUSCPPGLIB_INCLUDE_DIR}
	${GLIBMM_INCLUDE_DIRS}
	${LXC_INCLUDE_DIRS}
)

option(ENABLE_TEST "Enables unit testing" ON)
if(ENABLE_TEST)
	enable_testing()
	add_gmock()
endif()

add_coverage()

install(FILES scripts/setup_pelagicontain.sh DESTINATION bin PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)

add_subdirectory(pelagicontain)
add_subdirectory(controller)
add_subdirectory(doc)
