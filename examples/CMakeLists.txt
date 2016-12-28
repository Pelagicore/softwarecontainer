
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

# Unset RPATH. This should not be needed for any example.
set(CMAKE_INSTALL_RPATH "")

pkg_check_modules(DBusCpp REQUIRED dbus-c++-glib-1>=0.9.0)

add_definitions(${DBusCpp_CFLAGS_OTHER})
include_directories(${DBusCpp_INCLUDE_DIRS})
link_directories(${DBusCpp_LIBRARY_DIRS})

add_subdirectory(simple)
add_subdirectory(temperature)
add_subdirectory(wayland)
