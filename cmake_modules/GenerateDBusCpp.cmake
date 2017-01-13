#
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
#

macro(generate_dbuscpp_hfile xmlInterfaceFile h)

    set(DBUS_GENERATED_DIR ${CMAKE_CURRENT_BINARY_DIR}/GeneratedDBusCode)
    file(MAKE_DIRECTORY ${DBUS_GENERATED_DIR})

    include_directories(${DBUS_GENERATED_DIR})

    set(GENERATED_PROXY_FILENAME ${DBUS_GENERATED_DIR}/${h}_dbuscpp_proxy.h)
    set(GENERATED_ADAPTOR_FILENAME ${DBUS_GENERATED_DIR}/${h}_dbuscpp_adaptor.h)

    message(STATUS "Creating DBusCpp proxy and stubs : ${GENERATED_PROXY_FILENAME} / ${GENERATED_ADAPTOR_FILENAME}")

    add_custom_command(
        OUTPUT ${GENERATED_PROXY_FILENAME} ${GENERATED_ADAPTOR_FILENAME}
        COMMAND dbusxx-xml2cpp ${xmlInterfaceFile} --proxy=${GENERATED_PROXY_FILENAME}
        COMMAND dbusxx-xml2cpp ${xmlInterfaceFile} --adaptor=${GENERATED_ADAPTOR_FILENAME}
        DEPENDS ${xmlInterfaceFile}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )

    add_custom_target(dbus_cpp_${h} ALL DEPENDS ${GENERATED_PROXY_FILENAME} ${GENERATED_ADAPTOR_FILENAME})

endmacro()
