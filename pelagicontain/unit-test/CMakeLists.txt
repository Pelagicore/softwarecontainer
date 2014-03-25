
add_definitions(${DEPENDENCIES_CFLAGS})

# Compile an executable that will run your unit tests and link it with gtest and gtest_main:
add_executable(runUnitTestsPelagicontain
    devicenodegateway_unittest.cpp
    devicenodegateway_unittest_data.h
    main.cpp
    networkgateway_unittest.cpp
    pelagicontain_unittest.cpp
    ../src/container.cpp
    ../src/controllerinterface.cpp
    ../src/dbusgateway.cpp
    ../src/devicenodegateway.cpp
    ../src/generators.cpp
    ../src/networkgateway.cpp
    ../src/pelagicontain.cpp
    ../src/pulsegateway.cpp
)

link_directories(
	${DEPENDENCIES_LIBRARY_DIRS} ${PELAGICORE_UTILS_LIBRARY_DIRS}
)

TARGET_LINK_LIBRARIES(runUnitTestsPelagicontain
	${PELAGICORE_UTILS_LIBRARIES}
	${DEPENDENCIES_LIBRARIES}
	${DBUS_LIBRARIES} ${DBUSCPP_LIBRARIES}
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
