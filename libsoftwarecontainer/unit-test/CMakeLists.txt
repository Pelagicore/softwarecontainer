
set(TEST_DATA_DIR ${CMAKE_CURRENT_SOURCE_DIR}/test-data)
add_definitions(-DTEST_DATA_DIR="${TEST_DATA_DIR}")

set(TEST_LIBRARY_DEPENDENCIES 
    ${PELAGICORE_UTILS_LIBRARIES}
    ${DEPENDENCIES_LIBRARIES}
    ${DBUS_LIBRARIES}
    ${DBUSCPP_LIBRARIES}
    ${DBUSCPPGLIB_LIBRARIES}
    ${GLIBMM_LIBRARIES}
    ${LXC_LIBRARIES}
    softwarecontainerLib
)

include_directories(${LIBPELAGICONTAIN_DIR}/include)
link_directories(
    ${DEPENDENCIES_LIBRARY_DIRS}
    ${PELAGICORE_UTILS_LIBRARY_DIRS}
)

set(TEST_FILES
    main.cpp
    softwarecontainer_test.cpp
    softwarecontainerlib_unittest.cpp
    cgroupsgateway_unittest.cpp
    filegateway_unittest.cpp
    envgateway_unittest.cpp
    dbusgateway_unittest.cpp
)

add_gtest_test(softwarecontainerLibTest
    "${TEST_FILES}"
    "${TEST_LIBRARY_DEPENDENCIES}"
)
