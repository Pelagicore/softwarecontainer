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

include (AddGTestTest)

set(TEST_DATA_DIR ${CMAKE_CURRENT_SOURCE_DIR}/test-data)
add_definitions(-DTEST_DATA_DIR="${TEST_DATA_DIR}")

include_directories(
    ${LIBSOFTWARECONTAINER_DIR}/src
    ${LIBSOFTWARECONTAINER_DIR}/src/config
)

set(TEST_LIBRARY_DEPENDENCIES
    ${IVILogging_LIBRARIES}
    rt # For SHM tests to work
    softwarecontainer
)

set(GATEWAY_TEST_FILES "")
macro(add_gateway_test GATEWAY_FLAG FILENAME)
    if (${GATEWAY_FLAG})
        LIST(APPEND GATEWAY_TEST_FILES ${FILENAME})
    endif()
endmacro()

add_gateway_test(ENABLE_DBUSGATEWAY dbusgateway_componenttest.cpp)
add_gateway_test(ENABLE_FILEGATEWAY filegateway_componenttest.cpp)
add_gateway_test(ENABLE_NETWORKGATEWAY netlink_componenttest.cpp)
add_gateway_test(ENABLE_NETWORKGATEWAY networkgateway_componenttest.cpp)

set(TEST_FILES
    softwarecontainer_test.cpp
    softwarecontainerlib_componenttest.cpp
    ${LIBSOFTWARECONTAINER_DIR}/src/config/softwarecontainerconfig.cpp
    ${GATEWAY_TEST_FILES}
    main.cpp
)

add_gtest_test(softwarecontainer-component-test
    "${TEST_FILES}"
    "${TEST_LIBRARY_DEPENDENCIES}"
)
