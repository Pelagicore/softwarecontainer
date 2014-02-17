FIND_PACKAGE(PelagicoreUtilities REQUIRED)
add_gmock()
add_coverage()

add_definitions(${DEPENDENCIES_CFLAGS})

# Compile an executable that will run your unit tests and link it with gtest and gtest_main:
add_executable(runUnitTests
    pelagicontain_unittest.cpp
    ../src/pelagicontain.cpp
    ../src/container.cpp
    ../src/networkgateway.cpp
    ../src/dbusgateway.cpp
    ../src/pulsegateway.cpp
    ../src/controllerinterface.cpp
    ../src/generators.cpp
)

link_directories(
	${DEPENDENCIES_LIBRARY_DIRS} ${PELAGICORE_UTILS_LIBRARY_DIRS}
)

TARGET_LINK_LIBRARIES(runUnitTests
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
    NAME runUnitTests
    COMMAND runUnitTests
)

add_definitions(-DTESTDATA_PATH="${CMAKE_CURRENT_SOURCE_DIR}/test-data")
