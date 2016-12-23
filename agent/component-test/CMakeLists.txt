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

set(AGENT_TEST_LIBRARY_DEPENDENCIES
    ${DBusCpp_LIBRARIES}
    ${GLibmm_LIBRARIES}
    ${IVILogging_LIBRARIES}
    softwarecontainer
)

set(SOFTWARECONTAINERAGENT_TEST_FILES
    main.cpp
    softwarecontaineragent_unittest.cpp
    ${SOFTWARECONTAINERAGENT_DIR}/src/softwarecontaineragent.cpp
    ${SOFTWARECONTAINERAGENT_DIR}/src/softwarecontaineragentadaptor.cpp
    ${SOFTWARECONTAINERAGENT_DIR}/src/capability/baseconfigstore.cpp
    ${SOFTWARECONTAINERAGENT_DIR}/src/capability/defaultconfigstore.cpp
    ${SOFTWARECONTAINERAGENT_DIR}/src/capability/filteredconfigstore.cpp
    ${SOFTWARECONTAINERAGENT_DIR}/src/config/config.cpp
    ${SOFTWARECONTAINERAGENT_DIR}/src/config/fileconfigloader.cpp
    ${SOFTWARECONTAINERAGENT_DIR}/src/config/configdefaults.cpp
    ${SOFTWARECONTAINERAGENT_DIR}/src/config/mandatoryconfigs.cpp
)

add_gtest_test(softwarecontaineragent-component-test
    "${SOFTWARECONTAINERAGENT_TEST_FILES}"
    "${AGENT_TEST_LIBRARY_DEPENDENCIES}"
)
