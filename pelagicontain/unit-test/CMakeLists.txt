
add_definitions(${DEPENDENCIES_CFLAGS})

SET(GATEWAY_TEST_SRC "")
# It would be cool to be able to pass several files to this macro
macro(gateway_support_unit flag filename)
    if(${flag})
        LIST(APPEND GATEWAY_TEST_SRC ${filename})
    endif()
endmacro()

gateway_support_unit(ENABLE_PULSEGATEWAY pulsegateway_unittest.cpp)
gateway_support_unit(ENABLE_PULSEGATEWAY pulsegateway_unittest_data.h)
gateway_support_unit(ENABLE_PULSEGATEWAY ../src/pulsegateway.cpp)

gateway_support_unit(ENABLE_NETWORKGATEWAY networkgateway_unittest.cpp)
gateway_support_unit(ENABLE_NETWORKGATEWAY ../src/networkgateway.cpp)

gateway_support_unit(ENABLE_DBUSGATEWAY dbusgateway_unittest.cpp)
gateway_support_unit(ENABLE_DBUSGATEWAY ../src/dbusgateway.cpp)

gateway_support_unit(ENABLE_DEVICENODEGATEWAY devicenodegateway_unittest.cpp)
gateway_support_unit(ENABLE_DEVICENODEGATEWAY devicenodegateway_unittest_data.h)
gateway_support_unit(ENABLE_DEVICENODEGATEWAY ../src/devicenodegateway.cpp)

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

link_directories(
	${DEPENDENCIES_LIBRARY_DIRS} ${PELAGICORE_UTILS_LIBRARY_DIRS}
)

# Compile an executable that will run your unit tests and link it with gtest and gtest_main:
#add_gtest_test(pelagicontainUnitTest
#    "main.cpp;pelagicontain_unittest.cpp;generators_unittest.cpp;${GATEWAY_TEST_SRC}"
#    "${TEST_LIBRARY_DEPENDENCIES}"
#)
# TODO : rework tests

add_gtest_test(pelagicontainLibTest
    "main.cpp;pelagicontainLib_unittest.cpp"
    "${TEST_LIBRARY_DEPENDENCIES}"
)
