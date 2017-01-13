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

macro(add_gtest_test testName files libraries)

    if ((NOT TARGET gtest) OR (NOT TARGET gmock))
        message(FATAL_ERROR "No gtest or gmock targets found, can't add tests")
    else()
        add_executable(${testName}
            ${files}
        )

        target_link_libraries(${testName}
            ${libraries}
            gtest
            gtest_main
            gmock
            gmock_main
        )

        add_test(
            NAME ${testName}
            COMMAND ${testName}
        )
    endif()

endmacro()
