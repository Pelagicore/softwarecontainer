
generate_dbuscpp_hfile(${CMAKE_CURRENT_SOURCE_DIR}/pelagicontain-agent.xml "PelagicontainAgent")

add_executable(pelagicontain-agent
    src/pelagicontainagent.cpp
    PelagicontainAgent_dbuscpp_adaptor.h
)

target_link_libraries( pelagicontain-agent
    ${PELAGICORE_UTILS_LIBRARIES}
    ${DEPENDENCIES_LIBRARIES}
    ${DBUS_LIBRARIES}
    ${DBUSCPP_LIBRARIES}
    ${DBUSCPPGLIB_LIBRARIES}
    ${GLIBMM_LIBRARIES}
    ${LXC_LIBRARIES}
    ${COMMON_DEPENDENCIES_LIBRARIES}
    pelagicontainLib
)

install(TARGETS pelagicontain-agent DESTINATION bin)

# Install the D-Bus configuration file
if (${CMAKE_INSTALL_PREFIX} STREQUAL "/usr")
    set(DBUS_SYSTEM_BUS_CONFIGURATION_FILES_DIR "/usr/share/dbus-1/system.d/")
else()
    set(DBUS_SYSTEM_BUS_CONFIGURATION_FILES_DIR "/etc/dbus-1/system.d")
endif()
install(FILES pelagicontain-agent.conf DESTINATION ${DBUS_SYSTEM_BUS_CONFIGURATION_FILES_DIR})

# Install systemd configuration
if (ENABLE_SYSTEMD)
    message("SystemD file installation enabled")

    if (${CMAKE_INSTALL_PREFIX} STREQUAL "/usr")
        set(SYSTEMD_CONFIGURATION_FILES_DIR "/lib/systemd/system")
    else()
        set(SYSTEMD_CONFIGURATION_FILES_DIR "systemd/system")
    endif()
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/pelagicontain-agent.service.in  ${CMAKE_CURRENT_BINARY_DIR}/pelagicontain-agent.service)
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/pelagicontain-agent.service DESTINATION ${SYSTEMD_CONFIGURATION_FILES_DIR})
endif()
