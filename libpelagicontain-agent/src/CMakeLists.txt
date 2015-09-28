
generate_dbuscpp_hfile(${CMAKE_SOURCE_DIR}/agent/pelagicontain-agent.xml "PelagicontainAgentLib")

add_library(pelagicontain-agent-lib SHARED
    libpelagicontainagent.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/PelagicontainAgentLib_dbuscpp_proxy.h
)

TARGET_LINK_LIBRARIES( pelagicontain-agent-lib
    ${PELAGICORE_UTILS_LIBRARIES}
    ${DBUSCPP_LIBRARIES}
    ${DBUSCPPGLIB_LIBRARIES}
    ${GLIBMM_LIBRARIES}
)

set(HEADERS
    libpelagicontainagent.h
)

install(FILES ${HEADERS} DESTINATION include/pelagicontain)
install(TARGETS pelagicontain-agent-lib DESTINATION lib)

set_target_properties(pelagicontain-agent-lib PROPERTIES VERSION ${VERSION} SOVERSION ${${PROJECT_NAME}_MAJOR_VERSION})

