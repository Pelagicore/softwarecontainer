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

include(GNUInstallDirs)
include(Configs)
include(GenerateDBusCpp)
generate_dbuscpp_hfile(${CMAKE_CURRENT_SOURCE_DIR}/softwarecontainer-agent.xml "SoftwareContainerAgent")

install(DIRECTORY DESTINATION ${SC_SERVICE_MANIFEST_DIR})
install(DIRECTORY DESTINATION ${SC_DEFAULT_SERVICE_MANIFEST_DIR})

include_directories(src)

#
# Build the softwarecontainer-agent binary
#
add_executable(softwarecontainer-agent
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/softwarecontaineragent_dbus_common.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/softwarecontaineragent_dbus_stub.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/softwarecontaineragent.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/softwarecontaineragentadaptor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/capability/baseconfigstore.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/capability/defaultconfigstore.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/capability/filteredconfigstore.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/config/config.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/config/fileconfigloader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/config/commandlineconfigsource.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/config/mainconfigsource.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/config/defaultconfigsource.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/config/configitem.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/config/configdefinition.cpp
    ${GENERATED_ADAPTOR_FILENAME}
)

target_link_libraries(softwarecontainer-agent
    ${Giomm_LIBRARIES}
    ${IVILogging_LIBRARIES}
    softwarecontainer
)

install(TARGETS softwarecontainer-agent DESTINATION bin)

# At this point, there should be no more definitions being added/changed so it's OK to set them
set_all_deferred_config_definitions()

# Configure and install the general config file
configure_file(softwarecontainer-config.in softwarecontainer-config)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/softwarecontainer-config" DESTINATION ${SYS_CONFIG_DIR})

#
# When we install systemd config files, we check for if prefix is /usr, and we treat
# this as installing system-wide, which means some config files should also
# be installed system-wide to other locations than they would otherwise have
# been installed to.
#
if (ENABLE_SYSTEM_BUS)
    # Install the D-Bus configuration file
    # This enables the agent to communicate over the system bus
    install(FILES softwarecontainer-agent.conf DESTINATION "/etc/dbus-1/system.d")
endif()

# Install systemd configuration
if (ENABLE_SYSTEMD)
    set(SYSTEMD_CONFIGURATION_FILES_DIR "${CMAKE_INSTALL_FULL_LIBDIR}/systemd/system")
    configure_file(softwarecontainer-agent.service.in softwarecontainer-agent.service)
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/softwarecontainer-agent.service
            DESTINATION ${SYSTEMD_CONFIGURATION_FILES_DIR}
    )
endif()

if (ENABLE_TEST)
    set(SOFTWARECONTAINERAGENT_DIR ${CMAKE_CURRENT_SOURCE_DIR})
    add_subdirectory(unit-test)
    add_subdirectory(component-test)
endif()
