
add_definitions(${DEPENDENCIES_CFLAGS})

# Compile an executable that will run your unit tests and link it with gtest and gtest_main:
add_executable(runUnitTestsController
    main.cpp
)

link_directories(
        ${DEPENDENCIES_LIBRARY_DIRS} ${PELAGICORE_UTILS_LIBRARY_DIRS}
)

TARGET_LINK_LIBRARIES(runUnitTestsController
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
    NAME runUnitTestsController
    COMMAND runUnitTestsController
)

add_definitions(-DTESTDATA_PATH="${CMAKE_CURRENT_SOURCE_DIR}/test-data")
