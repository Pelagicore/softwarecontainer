
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


add_definitions(-DLXCTEMPLATE="${LXC_TEMPLATE_DIR}/lxc-softwarecontainer")

set(LIBSOFTWARECONTAINER_DIR ${CMAKE_CURRENT_SOURCE_DIR})
add_subdirectory(include)
add_subdirectory(src)

if(ENABLE_TEST)
    add_subdirectory(unit-test)
    add_subdirectory(component-test)
endif(ENABLE_TEST)

# busybox is currently always needed on the host system at runtime
find_program(busybox_binary busybox)
if(NOT busybox_binary)
    message(WARNING "'busybox' binary was not found.")
endif(NOT busybox_binary)

if(ENABLE_DBUSGATEWAY)
    # dbus-proxy binary needs to be available if the D-Bus gateway is enabled
    find_program(dbus-proxy_binary dbus-proxy)
    if(NOT dbus-proxy_binary)
        message(WARNING "'dbus-proxy' binary was not found. This is needed when D-Bus gateway is enabled.")
    endif(NOT dbus-proxy_binary)
endif(ENABLE_DBUSGATEWAY)

if(ENABLE_NETWORKGATEWAY)
    # brctl binary needs to be available at runtime if the network gateway is enabled
    find_program(brctl_binary brctl)
    if(NOT brctl_binary)
        message(WARNING "'brctl' binary was not found. This is needed when network gateway is enabled.")
    endif(NOT brctl_binary)

    # Read network config to be used below when configuring the general config file
    configure_file(softwarecontainer_network.conf.in softwarecontainer_network.conf)

    file(READ ${CMAKE_CURRENT_BINARY_DIR}/softwarecontainer_network.conf NETWORK_LXC_CONF)
endif(ENABLE_NETWORKGATEWAY)

configure_file(softwarecontainer.conf.in softwarecontainer.conf)
configure_file(lxc-softwarecontainer.in lxc-softwarecontainer)
configure_file(softwarecontainer.pc.in softwarecontainer.pc @ONLY)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/softwarecontainer.pc DESTINATION ${CMAKE_INSTALL_FULL_LIBDIR}/pkgconfig)
install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/lxc-softwarecontainer DESTINATION ${LXC_TEMPLATE_DIR})
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/softwarecontainer.conf DESTINATION ${SYS_CONFIG_DIR})
