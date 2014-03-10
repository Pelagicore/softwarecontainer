
add_definitions(${DEPENDENCIES_CFLAGS})

# Compile an executable that will run your unit tests and link it with gtest and gtest_main:
add_executable(runComponentTestsPelagicontain
    main.cpp
    pelagicontain_componenttest.cpp
    ../src/pelagicontain.cpp
    ../src/container.cpp
    ../src/controllerinterface.cpp
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

add_test(
    NAME runComponentTestsPelagicontain
    COMMAND runComponentTestsPelagicontain
)

add_definitions(-DTESTDATA_PATH="${CMAKE_CURRENT_SOURCE_DIR}/test-data")
