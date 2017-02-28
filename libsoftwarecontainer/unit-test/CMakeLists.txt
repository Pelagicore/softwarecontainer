
# Copyright (C) 2016-2017 Pelagicore AB
#
# Permission to use, copy, modify, and/or distribute this software for
# any purpose with or without fee is hereby granted, provided that the
# above copyright notice and this permission notice appear in all copies.
#
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
# WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR
# BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES
# OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
# WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
# ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
# SOFTWARE.
#
# For further information see LICENSE

include (AddGTestTest)

set(TEST_LIBRARY_DEPENDENCIES
    ${IVILogging_LIBRARIES}
    softwarecontainer
)

include_directories(
    ${LIBSOFTWARECONTAINER_DIR}/src
)

set(GATEWAY_TEST_FILES "")
macro(add_gateway_test GATEWAY_FLAG FILENAME)
    if (${GATEWAY_FLAG})
        LIST(APPEND GATEWAY_TEST_FILES ${FILENAME})
    endif()
endmacro()

add_gateway_test(ENABLE_CGROUPSGATEWAY cgroupsparser_unittest.cpp)

add_gateway_test(ENABLE_DBUSGATEWAY dbusgatewayparser_unittest.cpp)

add_gateway_test(ENABLE_DEVICENODEGATEWAY devicenodeparser_unittest.cpp)
add_gateway_test(ENABLE_DEVICENODEGATEWAY devicenodelogic_unittest.cpp)

add_gateway_test(ENABLE_ENVGATEWAY envgatewayparser_unittest.cpp)

add_gateway_test(ENABLE_FILEGATEWAY filegatewayparser_unittest.cpp)
add_gateway_test(ENABLE_FILEGATEWAY filegatewaysettingstore_unittest.cpp)

add_gateway_test(ENABLE_NETWORKGATEWAY iptableentry_unittest.cpp)
add_gateway_test(ENABLE_NETWORKGATEWAY networkgatewayparser_unittest.cpp)
add_gateway_test(ENABLE_NETWORKGATEWAY networkgatewayfunctions_unittest.cpp)

add_gateway_test(ENABLE_PULSEGATEWAY pulsegateway_unittest.cpp)

# We already add libsoftwarecontainer as a dependency above but some
# tests need to build towards to code directly so some cpp files
# are added here as well, even if libsoftwarecontainer already is
# built with them.
set(TEST_FILES
    gateway_unittest.cpp
    main.cpp
    ${GATEWAY_TEST_FILES}
    ${CMAKE_SOURCE_DIR}/libsoftwarecontainer/src/gateway/gateway.cpp
)

add_gtest_test(softwarecontainer-unit-test
    "${TEST_FILES}"
    "${TEST_LIBRARY_DEPENDENCIES}"
)
