
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

set(HEADERS
    softwarecontainer-common.h
    softwarecontainer-log.h
    softwarecontainererror.h
    jsonparser.h
    cleanuphandler.h
    overlaysynccleanuphandler.h
    recursivecopy.h
    recursivedelete.h
    directorycleanuphandler.h
    filecleanuphandler.h
    mountcleanuphandler.h
    createdir.h
    filetoolkitwithundo.h
    gatewayconfig.h
    signalconnectionshandler.h
)

add_library(softwarecontainercommon SHARED
    directorycleanuphandler.cpp
    filecleanuphandler.cpp
    jsonparser.cpp
    createdir.cpp
    filetoolkitwithundo.cpp
    mountcleanuphandler.cpp
    overlaysynccleanuphandler.cpp
    softwarecontainer-common.cpp
    recursivecopy.cpp
    recursivedelete.cpp
    gatewayconfig.cpp
    signalconnectionshandler.cpp
)

target_link_libraries(softwarecontainercommon
    ${Jansson_LIBRARIES}
    ${sigc_LIBRARIES})

install(FILES ${HEADERS} DESTINATION include/softwarecontainer)
install(TARGETS softwarecontainercommon DESTINATION lib)


if(ENABLE_TEST)
    set(SOFTWARECONTAINER_COMMON_DIR ${CMAKE_CURRENT_SOURCE_DIR})
    add_subdirectory(unit-test)
endif()
