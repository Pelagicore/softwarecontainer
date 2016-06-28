
set(TEST_LIBRARY_DEPENDENCIES 
    ${PELAGICORE_UTILS_LIBRARIES}
    ${DEPENDENCIES_LIBRARIES}
    ${DBUS_LIBRARIES}
    ${DBUSCPP_LIBRARIES}
    ${DBUSCPPGLIB_LIBRARIES}
    ${GLIBMM_LIBRARIES}
    ${LXC_LIBRARIES}
    pelagicontainLib
)

include_directories(${LIBPELAGICONTAIN_DIR}/include)
link_directories(
    ${DEPENDENCIES_LIBRARY_DIRS}
    ${PELAGICORE_UTILS_LIBRARY_DIRS}
)

add_gtest_test(pelagicontainLibTest
    "main.cpp;pelagicontainlib_unittest.cpp"
    "${TEST_LIBRARY_DEPENDENCIES}"
)
