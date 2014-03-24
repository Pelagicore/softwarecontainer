
add_definitions(${DEPENDENCIES_CFLAGS})

# Compile an executable that will run your unit tests and link it with gtest and gtest_main:
add_executable(runUnitTestsPelagicontain
    main.cpp
    pelagicontain_unittest.cpp
    networkgateway_unittest.cpp
    devicenodegateway_unittest.cpp
    ../src/pelagicontain.cpp
    ../src/container.cpp
    ../src/networkgateway.cpp
    ../src/devicenodegateway.cpp
    ../src/dbusgateway.cpp
    ../src/pulsegateway.cpp
    ../src/controllerinterface.cpp
    ../src/generators.cpp
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
