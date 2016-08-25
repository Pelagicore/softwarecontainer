cmake_minimum_required(VERSION 2.6)

project(softwarecontainer)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake_modules)

SET( ${PROJECT_NAME}_MAJOR_VERSION 0 )
SET( ${PROJECT_NAME}_MINOR_VERSION 9 )
SET( ${PROJECT_NAME}_PATCH_LEVEL 1 )

set(VERSION ${${PROJECT_NAME}_MAJOR_VERSION}.${${PROJECT_NAME}_MINOR_VERSION}.${${PROJECT_NAME}_PATCH_LEVEL})

if(EXTRA_PKG_CONFIG_PATH)
	set(ENV{PKG_CONFIG_PATH} ${EXTRA_PKG_CONFIG_PATH})
	message("Using PKG_CONFIG_PATH: " ${EXTRA_PKG_CONFIG_PATH})
endif(EXTRA_PKG_CONFIG_PATH)

add_definitions(-DPACKAGE_VERSION="${VERSION}")

find_package(IVILogging          REQUIRED)
find_package(PkgConfig           REQUIRED)

pkg_check_modules(IVILogging    REQUIRED ivi-logging)
pkg_check_modules(DBusCpp       REQUIRED dbus-c++-glib-1)
pkg_check_modules(Glibmm        REQUIRED glibmm-2.4)
pkg_check_modules(LXC           REQUIRED lxc)
pkg_check_modules(IVIMainLoop   REQUIRED ivi-main-loop)
pkg_check_modules(JSONParser    REQUIRED jsonparser)

add_definitions(${IVILogging_CFLAGS_OTHER})
add_definitions(${DBusCpp_CFLAGS_OTHER})
add_definitions(${Glibmm_CFLAGS_OTHER})
add_definitions(${LXC_CFLAGS_OTHER})
add_definitions(${IVIMainLoop_CFLAGS_OTHER})
add_definitions(${JSONParser_CFLAGS_OTHER})

include_directories(${IVILogging_INCLUDE_DIRS})
include_directories(${DBusCpp_INCLUDE_DIRS})
include_directories(${Glibmm_INCLUDE_DIRS})
include_directories(${LXC_INCLUDE_DIRS})
include_directories(${IVIMainLoop_INCLUDE_DIRS})
include_directories(${JSONParser_INCLUDE_DIRS})

link_directories(${IVILogging_LIBRARY_DIRS})
link_directories(${DBusCpp_LIBRARY_DIRS})
link_directories(${Glibmm_LIBRARY_DIRS})
link_directories(${LXC_LIBRARY_DIRS})
link_directories(${IVIMainLoop_LIBRARY_DIRS})
link_directories(${JSONParser_LIBRARY_DIRS})

# use, i.e. don't skip the full RPATH for the build tree
SET(CMAKE_SKIP_BUILD_RPATH  FALSE)

# when building, don't use the install RPATH already
# (but later on when installing)
SET(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)

SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")

# add the automatically determined parts of the RPATH
# which point to directories outside the build tree to the install RPATH
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

# the RPATH to be used when installing, but only if it's not a system directory
LIST(FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES "${CMAKE_INSTALL_PREFIX}/lib" isSystemDir)
IF("${isSystemDir}" STREQUAL "-1")
   SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
ENDIF("${isSystemDir}" STREQUAL "-1")

if(ADDITIONAL_LD_LIBRARY_PATH)
    add_definitions(-DADDITIONAL_LD_LIBRARY_PATH=${ADDITIONAL_LD_LIBRARY_PATH})
endif()

if(ADDITIONAL_PATH)
    add_definitions(-DADDITIONAL_PATH=${ADDITIONAL_PATH})
endif()

if(${CMAKE_INSTALL_PREFIX} STREQUAL "/usr")
	SET(SYS_CONFIG_DIR /etc)
	SET(LXC_TEMPLATE_DIR /var/hl/lxc-templates/)
else()
	SET(SYS_CONFIG_DIR ${CMAKE_INSTALL_PREFIX}/etc)
	SET(LXC_TEMPLATE_DIR ${CMAKE_INSTALL_PREFIX}/var/hl/lxc-templates)
endif()
message("Configuration files will be installed in " ${SYS_CONFIG_DIR})
message("LXC template will be installed in " ${LXC_TEMPLATE_DIR})

include_directories(
    common
    libsoftwarecontainer/src
    libsoftwarecontainer/include
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}/libsoftwarecontainer/src
)

option(ENABLE_PROFILING "Enables profiling support in the application" OFF)
if(ENABLE_PROFILING)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DPROFILING_ENABLED=1")
endif()

# Google Mock add (and fetch) macro
macro(add_gmock)
    set(GMOCKNAME gmock-1.7.0)
    if(NOT IS_DIRECTORY ${CMAKE_BINARY_DIR}/${GMOCKNAME})
        if(NOT EXISTS ${CMAKE_BINARY_DIR}/${GMOCKNAME}.zip)
            execute_process(COMMAND wget -P ${CMAKE_BINARY_DIR} https://googlemock.googlecode.com/files/${GMOCKNAME}.zip)
        endif()
        execute_process(COMMAND unzip ${CMAKE_BINARY_DIR}/${GMOCKNAME}.zip -d ${CMAKE_BINARY_DIR})
    endif()
    if(IS_DIRECTORY ${CMAKE_BINARY_DIR}/${GMOCKNAME})
        add_subdirectory(${CMAKE_BINARY_DIR}/${GMOCKNAME} ${CMAKE_BINARY_DIR}/gmock)
        link_directories(${CMAKE_BINARY_DIR}/${GMOCKNAME}/gmock)
        include_directories(${CMAKE_BINARY_DIR}/${GMOCKNAME}/include ${CMAKE_BINARY_DIR}/${GMOCKNAME}/gtest/include)
    endif()
endmacro()

option(ENABLE_TEST "Enables unit testing" ON)
if(ENABLE_TEST)
	enable_testing()
	add_gmock()
    configure_file(scripts/run-tests.sh run-tests.sh COPYONLY)
endif()

option(ENABLE_SYSTEMD "Enables installation of systemD unit files" ON)

# Macro for making it easy to use gcov/lcov
macro(add_coverage)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage -O0")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fprofile-arcs -ftest-coverage")
	find_program(LCOV lcov)
	find_program(GENHTML genhtml)
	if (LCOV AND GENHTML)
		add_custom_target(lcov)
		add_custom_command(
			TARGET lcov
			COMMAND mkdir -p coverage
			WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
		)

        # Run baseline coverage file, then remove unwanted files from baseline data
		add_custom_command(
			TARGET lcov
            # Create baseline
			COMMAND ${LCOV} --capture --initial --directory . --base-directory . --output-file coverage/baseline.info
            # Remove unwanted files from baseline data
			COMMAND ${LCOV} --remove coverage/baseline.info \"/usr*\" --no-checksum --directory . --base-directory . --output-file coverage/baseline.info
			COMMAND ${LCOV} --remove coverage/baseline.info \"gmock*\" --no-checksum --directory . --base-directory . --output-file coverage/baseline.info
			COMMAND ${LCOV} --remove coverage/baseline.info \"gtest*\" --no-checksum --directory . --base-directory . --output-file coverage/baseline.info
			COMMAND ${LCOV} --remove coverage/baseline.info \"unit-test*\" --no-checksum --directory . --base-directory . --output-file coverage/baseline.info
			WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
		)

        # Run the actual tests and stuff
		add_custom_command(
			TARGET lcov
		    # Run tests
			COMMAND make test
		    # Capture coverage data after tests have run
			COMMAND ${LCOV} --capture --directory . --base-directory . --output-file coverage/testrun.info
		    # Remove unwanted files from captured data
			COMMAND ${LCOV} --remove coverage/testrun.info \"/usr*\" --no-checksum --directory . --base-directory . --output-file coverage/testrun.info
			COMMAND ${LCOV} --remove coverage/testrun.info \"gmock*\" --no-checksum --directory . --base-directory . --output-file coverage/testrun.info
			COMMAND ${LCOV} --remove coverage/testrun.info \"gtest*\" --no-checksum --directory . --base-directory . --output-file coverage/testrun.info
			COMMAND ${LCOV} --remove coverage/testrun.info \"unit-test*\" --no-checksum --directory . --base-directory . --output-file coverage/testrun.info
			WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
		)
		add_custom_command(
			TARGET lcov
		    # Combine baseline data and coverage data for accurate coverage stats
			COMMAND ${LCOV} --add-tracefile coverage/baseline.info --add-tracefile coverage/testrun.info --output-file coverage/final.info
		    # Generate HTML report
			COMMAND genhtml --no-prefix coverage/final.info --output-directory coverage/
			WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
		)
	else(LCOV AND GENHTML)
		message(WARNING "Please install lcov and genhtml to enable the lcov target (apt-get install lcov)" )
	endif(LCOV AND GENHTML)
	message("Code coverage enabled")
endmacro(add_coverage)

macro(generate_dbuscpp_hfile xmlInterfaceFile h)
	include_directories(${CMAKE_CURRENT_BINARY_DIR})

	set(GENERATED_PROXY_FILENAME ${h}_dbuscpp_proxy.h)
	set(GENERATED_ADAPTOR_FILENAME ${h}_dbuscpp_adaptor.h)

	message("Creating DBusCpp proxy and stubs : ${GENERATED_PROXY_FILENAME} / ${GENERATED_ADAPTOR_FILENAME}")

    add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${GENERATED_PROXY_FILENAME} ${CMAKE_CURRENT_BINARY_DIR}/${GENERATED_ADAPTOR_FILENAME}
        COMMAND dbusxx-xml2cpp ${xmlInterfaceFile} --proxy=${GENERATED_PROXY_FILENAME}
        COMMAND dbusxx-xml2cpp ${xmlInterfaceFile} --adaptor=${GENERATED_ADAPTOR_FILENAME}
        DEPENDS ${xmlInterfaceFile}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )

    add_custom_target(dbus_cpp_${h} ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${GENERATED_PROXY_FILENAME} ${CMAKE_CURRENT_BINARY_DIR}/${GENERATED_ADAPTOR_FILENAME})

endmacro()

if(ENABLE_COVERAGE)
    add_coverage()
endif(ENABLE_COVERAGE)

install(FILES scripts/setup_softwarecontainer.sh DESTINATION bin PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)

add_definitions(-DINSTALL_PREFIX="${CMAKE_INSTALL_PREFIX}")

configure_file(softwarecontainer.pc.in softwarecontainer.pc @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/softwarecontainer.pc DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/pkgconfig)

set(DEVELOPMENT_INCLUDE_PATH " -I${CMAKE_CURRENT_SOURCE_DIR}/agent/lib -I${CMAKE_CURRENT_SOURCE_DIR}/clientLib -I${CMAKE_CURRENT_BINARY_DIR} -I${CMAKE_CURRENT_SOURCE_DIR}/softwarecontainer/src -I${CMAKE_CURRENT_SOURCE_DIR}/common #")
set(DEVELOPMENT_LIBRARY_PATH " -L${CMAKE_CURRENT_BINARY_DIR}/clientLib -L${CMAKE_CURRENT_BINARY_DIR}/lib #" )
configure_file(softwarecontainer.pc.in softwarecontainer-uninstalled.pc @ONLY)
if(INSTALL_PKGCONFIG_UNINSTALLED_FILE)
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/softwarecontainer-uninstalled.pc DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/pkgconfig)
endif()

configure_file(softwarecontainer-config.h.in softwarecontainer-config.h @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/softwarecontainer-config.h DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/softwarecontainer)

add_subdirectory(common)
add_subdirectory(libsoftwarecontainer)

add_subdirectory(agent)
add_subdirectory(libsoftwarecontainer-agent)

option (ENABLE_EXAMPLES "Enable building and installation of examples" OFF)
if (ENABLE_EXAMPLES)
    add_subdirectory(examples)
endif()

add_subdirectory(doc)
