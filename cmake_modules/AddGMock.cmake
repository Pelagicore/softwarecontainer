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

# Google Mock add (and fetch) macro
macro(add_gmock)
    set(ZIPNAME release-1.8.0)
    set(ZIPFILE ${CMAKE_CURRENT_BINARY_DIR}/${ZIPNAME}.zip)
    set(MOCKDIR ${CMAKE_CURRENT_BINARY_DIR}/googletest-${ZIPNAME})

    if(NOT IS_DIRECTORY ${MOCKDIR})
        if(NOT EXISTS ${ZIPFILE})
            message(STATUS "Downloading googletest...")
            execute_process(COMMAND wget -nv -P ${CMAKE_CURRENT_BINARY_DIR} https://github.com/google/googletest/archive/${ZIPNAME}.zip)
        endif()
        message(STATUS "Extracting ${ZIPNAME}...")
        execute_process(COMMAND unzip -q ${ZIPFILE} -d ${CMAKE_CURRENT_BINARY_DIR})
    endif()

    if(IS_DIRECTORY ${MOCKDIR})
        add_subdirectory(${MOCKDIR} EXCLUDE_FROM_ALL)
    else()
        message(FATAL_ERROR "No ${MOCKDIR} found, can't add googletest")
    endif()
endmacro()

