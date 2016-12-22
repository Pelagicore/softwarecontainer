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

set(CONFIGSTORE_TEST_LIBRARY_DEPENDENCIES
    ${IVILogging_LIBRARIES}
    softwarecontainer
    softwarecontainercommon
)

set(CONFIGSTORE_TEST_FILES
    main.cpp
    configstore_unittest.cpp
    config_unittest.cpp
    ${SOFTWARECONTAINERAGENT_DIR}/src/capability/baseconfigstore.cpp
    ${SOFTWARECONTAINERAGENT_DIR}/src/capability/filteredconfigstore.cpp
    ${SOFTWARECONTAINERAGENT_DIR}/src/capability/defaultconfigstore.cpp
    ${SOFTWARECONTAINERAGENT_DIR}/src/config/config.cpp
    ${SOFTWARECONTAINERAGENT_DIR}/src/config/fileconfigloader.cpp
    ${SOFTWARECONTAINERAGENT_DIR}/src/config/mainconfigsource.cpp
    ${SOFTWARECONTAINERAGENT_DIR}/src/config/configitem.cpp
)

add_gtest_test(softwarecontaineragent-unit-test
    "${CONFIGSTORE_TEST_FILES}"
    "${CONFIGSTORE_TEST_LIBRARY_DEPENDENCIES}"
)
