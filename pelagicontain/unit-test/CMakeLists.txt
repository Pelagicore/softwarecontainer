
add_definitions(${DEPENDENCIES_CFLAGS})

SET(GATEWAY_TEST_SRC "")
# It would be cool to be able to pass several files to this macro
macro(gateway_support_unit flag filename)
    if(${flag})
        LIST(APPEND GATEWAY_TEST_SRC ${filename})
    endif(${flag})
endmacro(gateway_support_unit flag filename)

gateway_support_unit(ENABLE_PULSEGW pulsegateway_unittest.cpp)
gateway_support_unit(ENABLE_PULSEGW pulsegateway_unittest_data.h)
gateway_support_unit(ENABLE_PULSEGW ../src/pulsegateway.cpp)

gateway_support_unit(ENABLE_NETGW networkgateway_unittest.cpp)
gateway_support_unit(ENABLE_NETGW ../src/networkgateway.cpp)

gateway_support_unit(ENABLE_DBUSGW dbusgateway_unittest.cpp)
gateway_support_unit(ENABLE_DBUSGW ../src/dbusgateway.cpp)

gateway_support_unit(ENABLE_DEVICENODEGW devicenodegateway_unittest.cpp)
gateway_support_unit(ENABLE_DEVICENODEGW devicenodegateway_unittest_data.h)
gateway_support_unit(ENABLE_DEVICENODEGW ../src/devicenodegateway.cpp)

# Compile an executable that will run your unit tests and link it with gtest and gtest_main:
add_executable(runUnitTestsPelagicontain
    main.cpp
    pelagicontain_unittest.cpp
    generators_unittest.cpp
    configparser_unittest.cpp
    ../src/container.cpp
    ../src/controllerinterface.cpp
    ../src/generators.cpp
    ../src/configparser.cpp
    ../src/pelagicontain.cpp
    
    ${PROJECT_SOURCE_DIR}/common/config.cpp
    ${GATEWAY_TEST_SRC}
)

link_directories(
	${DEPENDENCIES_LIBRARY_DIRS} ${PELAGICORE_UTILS_LIBRARY_DIRS}
)

TARGET_LINK_LIBRARIES(runUnitTestsPelagicontain
	${PELAGICORE_UTILS_LIBRARIES}
	${DEPENDENCIES_LIBRARIES}
	${DBUS_LIBRARIES}
	${DBUSCPP_LIBRARIES}
	${DBUSCPPGLIB_LIBRARIES}
	${GLIBMM_LIBRARIES}
	gtest
	gtest_main
	gmock
	gmock_main
)

include_directories(../src)

add_test(
    NAME runUnitTestsPelagicontain
    COMMAND runUnitTestsPelagicontain
)

add_definitions(-DTESTDATA_PATH="${CMAKE_CURRENT_SOURCE_DIR}/test-data")
