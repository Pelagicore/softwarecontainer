
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



generate_dbuscpp_hfile(${CMAKE_SOURCE_DIR}/agent/softwarecontainer-agent.xml "SoftwareContainerAgentLib")

add_library(softwarecontainer-agent-lib SHARED
    libsoftwarecontaineragent.cpp
    ../include/libsoftwarecontaineragent.h
    ${CMAKE_CURRENT_BINARY_DIR}/SoftwareContainerAgentLib_dbuscpp_proxy.h
)

TARGET_LINK_LIBRARIES( softwarecontainer-agent-lib
    ${PELAGICORE_UTILS_LIBRARIES}
    ${DBUSCPP_LIBRARIES}
    ${DBUSCPPGLIB_LIBRARIES}
    ${GLIBMM_LIBRARIES}
)

include_directories(${LIBSOFTWARECONTAINERAGENT_DIR}/include)

install(TARGETS softwarecontainer-agent-lib DESTINATION lib)
set_target_properties(softwarecontainer-agent-lib PROPERTIES VERSION ${VERSION} SOVERSION ${${PROJECT_NAME}_MAJOR_VERSION})

