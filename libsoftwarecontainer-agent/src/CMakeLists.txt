
generate_dbuscpp_hfile(${CMAKE_SOURCE_DIR}/agent/softwarecontainer-agent.xml "SoftwareContainerAgentLib")

add_library(softwarecontainer-agent-lib SHARED
    libsoftwarecontaineragent.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/SoftwareContainerAgentLib_dbuscpp_proxy.h
)

TARGET_LINK_LIBRARIES( softwarecontainer-agent-lib
    ${PELAGICORE_UTILS_LIBRARIES}
    ${DBUSCPP_LIBRARIES}
    ${DBUSCPPGLIB_LIBRARIES}
    ${GLIBMM_LIBRARIES}
)

include_directories(${LIBPELAGICONTAINAGENT_DIR}/include)

install(TARGETS softwarecontainer-agent-lib DESTINATION lib)
set_target_properties(softwarecontainer-agent-lib PROPERTIES VERSION ${VERSION} SOVERSION ${${PROJECT_NAME}_MAJOR_VERSION})

