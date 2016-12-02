
# Copyright (C) 2016 Pelagicore AB
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

include(AddGTestTest)

set(TEST_DATA_DIR ${CMAKE_CURRENT_SOURCE_DIR}/test-data)
add_definitions(-DTEST_DATA_DIR="${TEST_DATA_DIR}")

set(AGENT_TEST_LIBRARY_DEPENDENCIES
    ${DBusCpp_LIBRARIES}
    ${Glibmm_LIBRARIES}
    ${LXC_LIBRARIES}
    ${Jansson_LIBRARIES}
    ${IVILogging_LIBRARIES}
    softwarecontainerlib
    softwarecontainercommon
)

set(CONFIGSTORE_TEST_LIBRARY_DEPENDENCIES
    ${Glibmm_LIBRARIES}
    ${Jansson_LIBRARIES}
    ${IVILogging_LIBRARIES}
    softwarecontainerlib
    softwarecontainercommon
)

include_directories(${SOFTWARECONTAINERAGENT_DIR}/src ${SOFTWARECONTAINERAGENT_DIR}/src/capability)

set(SOFTWARECONTAINERAGENT_TEST_FILES
    main.cpp
    softwarecontaineragent_unittest.cpp
    ${SOFTWARECONTAINERAGENT_CORE_CLASSES}
)

set(CONFIGSTORE_TEST_FILES
    main.cpp
    configstore_unittest.cpp
    ${SOFTWARECONTAINERAGENT_CORE_CLASSES}
)


add_gtest_test(softwarecontaineragenttest
    "${SOFTWARECONTAINERAGENT_TEST_FILES}"
    "${AGENT_TEST_LIBRARY_DEPENDENCIES}"
)

add_gtest_test(configstoretest
    "${CONFIGSTORE_TEST_FILES}"
    "${CONFIGSTORE_TEST_LIBRARY_DEPENDENCIES}"
)
