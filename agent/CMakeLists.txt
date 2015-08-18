
generate_dbuscpp_hfile(${CMAKE_CURRENT_SOURCE_DIR}/pelagicontain-agent.xml "PelagicontainAgent")

add_executable(pelagicontain-agent
	pelagicontain-agent.cpp
	PelagicontainAgent_dbuscpp_adaptor.h
)

TARGET_LINK_LIBRARIES( pelagicontain-agent
    ${PELAGICORE_UTILS_LIBRARIES}
    ${DEPENDENCIES_LIBRARIES}
    ${DBUS_LIBRARIES} ${DBUSCPP_LIBRARIES}
    ${DBUSCPPGLIB_LIBRARIES}
    ${GLIBMM_LIBRARIES}
    ${LXC_LIBRARIES}
    ${COMMON_DEPENDENCIES_LIBRARIES}
    pelagicontainLib
)

install(TARGETS pelagicontain-agent DESTINATION bin)

set(HEADERS
    lib/PelagicontainAgentLib.h
)

install(FILES ${HEADERS} DESTINATION include/pelagicontain)


add_library(pelagicontain-agent-lib SHARED
    lib/PelagicontainAgentLib.cpp
    PelagicontainAgent_dbuscpp_proxy.h
)

TARGET_LINK_LIBRARIES( pelagicontain-agent-lib
    ${PELAGICORE_UTILS_LIBRARIES}
    ${DBUSCPP_LIBRARIES}
    ${DBUSCPPGLIB_LIBRARIES}
    ${GLIBMM_LIBRARIES}
)

set_target_properties(pelagicontain-agent-lib PROPERTIES VERSION ${VERSION} SOVERSION ${${PROJECT_NAME}_MAJOR_VERSION})

install(TARGETS pelagicontain-agent-lib DESTINATION lib)

if (ENABLE_SYSTEMD)

	add_definitions(-DENABLE_SYSTEMD)
	IF(${CMAKE_INSTALL_PREFIX} STREQUAL "/usr")
		set(SYSTEMD_CONFIGURATION_FILES_DIR "/lib/systemd/system")
		set(DBUS_SYSTEM_BUS_CONFIGURATION_FILES_DIR "/etc/dbus-1/system.d")
	ELSE()
		set(SYSTEMD_CONFIGURATION_FILES_DIR "systemd/system")
		set(DBUS_SYSTEM_BUS_CONFIGURATION_FILES_DIR "etc/dbus-1/system.d")
	ENDIF()
	
	message("SystemD file installation enabled")
	configure_file(${CMAKE_CURRENT_SOURCE_DIR}/pelagicontain-agent.service.in  ${CMAKE_CURRENT_BINARY_DIR}/pelagicontain-agent.service)
	INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/pelagicontain-agent.service DESTINATION ${SYSTEMD_CONFIGURATION_FILES_DIR})
	INSTALL(FILES pelagicontain-agent.conf DESTINATION ${DBUS_SYSTEM_BUS_CONFIGURATION_FILES_DIR})

endif()
