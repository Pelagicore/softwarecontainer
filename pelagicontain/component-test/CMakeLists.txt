
add_definitions(${DEPENDENCIES_CFLAGS})

file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/generated)

# This should really be in some shared cmake file
macro(generate_dbus_hfilex type xml h)
    find_program(DBUSBINDINGTOOLX "dbusxx-xml2cpp")
    if(DBUSBINDINGTOOLX)
        add_custom_command(
            OUTPUT ${h}
            COMMAND dbusxx-xml2cpp ${xml} --${type}=${h}
            DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${xml}
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        )
    else(DBUSBINDINGTOOLX)
        MESSAGE(FATAL_ERROR " Cannot find dbusxx-xml2cpp")
    endif(DBUSBINDINGTOOLX)
endmacro(generate_dbus_hfilex type xml h)

generate_dbus_hfilex("proxy" "../../../shared-service-interfaces/pam-service.xml"
                             "generated/pamproxy.h")

# Compile an executable that will run your unit tests and link it with gtest and gtest_main:
add_executable(runComponentTestsPelagicontain
    main.cpp
    pelagicontain_componenttest.cpp
    ../src/pelagicontain.cpp
    ../src/container.cpp
    ../src/controllerinterface.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/generated/pamproxy.h
)

link_directories(
	${DEPENDENCIES_LIBRARY_DIRS} ${PELAGICORE_UTILS_LIBRARY_DIRS}
)

TARGET_LINK_LIBRARIES(runComponentTestsPelagicontain
	${PELAGICORE_UTILS_LIBRARIES}
	${DEPENDENCIES_LIBRARIES}
	${DBUS_LIBRARIES} ${DBUSCPP_LIBRARIES}
	gtest
	gtest_main
	gmock
	gmock_main
)
include_directories(../src)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/generated)

add_definitions(-DTESTDATA_PATH="${CMAKE_CURRENT_SOURCE_DIR}/test-data")
